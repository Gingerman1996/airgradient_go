#include "log_storage.h"

#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_intr_alloc.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "spiffs.h"

#include <stdarg.h>
#include <string.h>

static const char *TAG = "log_store";

// W25N512GV SPI NAND configuration
// Shares SPI2_HOST bus with e-paper display
// E-paper must initialize the bus with MISO pin for NAND to work
static const spi_host_device_t kNandSpiHost = SPI2_HOST;
static const int kNandPinMosi = 25;
static const int kNandPinMiso = 24;
static const int kNandPinSclk = 23;
static const int kNandPinCs = 4;
static const int kNandClockHz = 10 * 1000 * 1000;
static const uint32_t kNandPageSize = 2048;
static const uint32_t kNandPagesPerBlock = 64;
static const uint32_t kNandBlockSize = kNandPageSize * kNandPagesPerBlock;
static const uint32_t kNandBlockCount = 512;
static const uint16_t kNandSpareOffset = 2048;

// W25N512GV commands
static const uint8_t kCmdReset = 0xFF;
static const uint8_t kCmdReadId = 0x9F;
static const uint8_t kCmdReadStatus = 0x0F;
static const uint8_t kCmdWriteEnable = 0x06;
static const uint8_t kCmdPageRead = 0x13;
static const uint8_t kCmdReadCache = 0x03;
static const uint8_t kCmdProgramLoad = 0x02;
static const uint8_t kCmdProgramExecute = 0x10;
static const uint8_t kCmdBlockErase = 0xD8;

static const uint8_t kStatusReg3 = 0xC0;
static const uint8_t kStatusBusyMask = 0x01;

static const size_t kLogRingbufSize = 16 * 1024;
static const uint32_t kLogFlushBytes = 4096;
static const uint32_t kLogMaxBytes = 8 * 1024 * 1024;

static spi_device_handle_t g_nand_spi = nullptr;
static SemaphoreHandle_t g_nand_lock = nullptr;
static uint16_t g_good_blocks[kNandBlockCount];
static uint32_t g_good_block_count = 0;

static spiffs g_fs = {};
static RingbufHandle_t g_log_ringbuf = nullptr;
static TaskHandle_t g_log_task = nullptr;
static TaskHandle_t g_mount_task = nullptr;
static spiffs_file g_log_file = -1;
static bool g_log_ready = false;
static bool g_spiffs_mounted = false;
static bool g_mount_started = false;
static vprintf_like_t g_orig_vprintf = nullptr;
static uint32_t g_log_max_bytes = kLogMaxBytes;

static uint8_t *g_spiffs_work = nullptr;
static uint8_t *g_spiffs_fds = nullptr;
static uint8_t *g_spiffs_cache = nullptr;

struct spiffs_nand_ctx_t {
  SemaphoreHandle_t lock;
};

static spiffs_nand_ctx_t g_spiffs_ctx = {};

static bool nand_lock(TickType_t timeout_ticks) {
  if (!g_nand_lock) {
    return false;
  }
  return xSemaphoreTake(g_nand_lock, timeout_ticks) == pdTRUE;
}

static void nand_unlock(void) {
  if (g_nand_lock) {
    xSemaphoreGive(g_nand_lock);
  }
}

static esp_err_t nand_transact(uint8_t cmd, uint32_t addr, uint8_t addr_bits,
                               const uint8_t *tx, size_t tx_len, uint8_t *rx,
                               size_t rx_len, uint8_t dummy_bits) {
  if (!g_nand_spi) {
    return ESP_ERR_INVALID_STATE;
  }

  spi_transaction_ext_t t = {};
  t.base.flags = SPI_TRANS_VARIABLE_CMD;
  t.command_bits = 8;
  t.base.cmd = cmd;

  if (addr_bits > 0) {
    t.base.flags |= SPI_TRANS_VARIABLE_ADDR;
    t.address_bits = addr_bits;
    t.base.addr = addr;
  }

  if (dummy_bits > 0) {
    t.base.flags |= SPI_TRANS_VARIABLE_DUMMY;
    t.dummy_bits = dummy_bits;
  }

  t.base.length = tx_len * 8;
  t.base.tx_buffer = tx;
  t.base.rxlength = rx_len * 8;
  t.base.rx_buffer = rx;

  return spi_device_polling_transmit(g_nand_spi, &t.base);
}

static esp_err_t nand_write_enable(void) {
  return nand_transact(kCmdWriteEnable, 0, 0, nullptr, 0, nullptr, 0, 0);
}

static esp_err_t nand_read_status(uint8_t reg_addr, uint8_t *out_status) {
  uint8_t rx[1] = {0};
  esp_err_t ret =
      nand_transact(kCmdReadStatus, reg_addr, 8, nullptr, 0, rx, sizeof(rx), 0);
  if (ret == ESP_OK) {
    *out_status = rx[0];
  }
  return ret;
}

static esp_err_t nand_wait_ready(uint32_t timeout_ms) {
  int64_t start_ms = esp_timer_get_time() / 1000;
  while (true) {
    uint8_t status = 0;
    esp_err_t ret = nand_read_status(kStatusReg3, &status);
    if (ret != ESP_OK) {
      return ret;
    }
    if ((status & kStatusBusyMask) == 0) {
      return ESP_OK;
    }
    if ((esp_timer_get_time() / 1000 - start_ms) > timeout_ms) {
      return ESP_ERR_TIMEOUT;
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

static esp_err_t nand_reset(void) {
  ESP_LOGI(TAG, "Sending NAND reset command (0xFF)...");
  esp_err_t ret = nand_transact(kCmdReset, 0, 0, nullptr, 0, nullptr, 0, 0);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "NAND reset transact failed: %s", esp_err_to_name(ret));
    return ret;
  }
  ESP_LOGI(TAG, "NAND reset sent, waiting for ready...");
  ret = nand_wait_ready(100);  // Increase timeout to 100ms
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "NAND wait_ready failed: %s", esp_err_to_name(ret));
  }
  return ret;
}

static esp_err_t nand_read_id(uint8_t *out_mfr, uint8_t *out_dev1,
                              uint8_t *out_dev2) {
  uint8_t rx[3] = {0};
  // W25N512 Read ID: CMD (0x9F) + 8-bit dummy address (0x00) + read 3 bytes
  esp_err_t ret =
      nand_transact(kCmdReadId, 0, 8, nullptr, 0, rx, sizeof(rx), 0);
  if (ret == ESP_OK) {
    *out_mfr = rx[0];
    *out_dev1 = rx[1];
    *out_dev2 = rx[2];
  }
  return ret;
}

static esp_err_t nand_page_read(uint32_t page) {
  esp_err_t ret = nand_transact(kCmdPageRead, page, 24, nullptr, 0, nullptr, 0, 0);
  if (ret != ESP_OK) {
    return ret;
  }
  return nand_wait_ready(5);
}

static esp_err_t nand_read_cache(uint16_t column, uint8_t *dst, size_t len) {
  return nand_transact(kCmdReadCache, column, 16, nullptr, 0, dst, len, 8);
}

static esp_err_t nand_read_page(uint32_t page, uint8_t *dst) {
  esp_err_t ret = nand_page_read(page);
  if (ret != ESP_OK) {
    return ret;
  }
  return nand_read_cache(0, dst, kNandPageSize);
}

static esp_err_t nand_program_page(uint32_t page, uint16_t column,
                                   const uint8_t *data, size_t len) {
  if (len == 0 || len > kNandPageSize) {
    return ESP_ERR_INVALID_SIZE;
  }

  esp_err_t ret = nand_write_enable();
  if (ret != ESP_OK) {
    return ret;
  }

  ret = nand_transact(kCmdProgramLoad, column, 16, data, len, nullptr, 0, 0);
  if (ret != ESP_OK) {
    return ret;
  }

  ret = nand_transact(kCmdProgramExecute, page, 24, nullptr, 0, nullptr, 0, 0);
  if (ret != ESP_OK) {
    return ret;
  }
  return nand_wait_ready(10);
}

static esp_err_t nand_erase_block(uint32_t page) {
  esp_err_t ret = nand_write_enable();
  if (ret != ESP_OK) {
    return ret;
  }

  ret = nand_transact(kCmdBlockErase, page, 24, nullptr, 0, nullptr, 0, 0);
  if (ret != ESP_OK) {
    return ret;
  }
  return nand_wait_ready(200);
}

static esp_err_t nand_read_bytes(uint32_t phys_addr, uint8_t *dst, size_t len) {
  while (len > 0) {
    uint32_t page = phys_addr / kNandPageSize;
    uint16_t column = phys_addr % kNandPageSize;
    size_t chunk = kNandPageSize - column;
    if (chunk > len) {
      chunk = len;
    }

    esp_err_t ret = nand_page_read(page);
    if (ret != ESP_OK) {
      return ret;
    }
    ret = nand_read_cache(column, dst, chunk);
    if (ret != ESP_OK) {
      return ret;
    }

    phys_addr += chunk;
    dst += chunk;
    len -= chunk;

    if ((phys_addr % (kNandPageSize * 8)) == 0) {
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
  return ESP_OK;
}

static esp_err_t nand_write_bytes(uint32_t phys_addr, const uint8_t *src,
                                  size_t len) {
  while (len > 0) {
    uint32_t page = phys_addr / kNandPageSize;
    uint16_t column = phys_addr % kNandPageSize;
    size_t chunk = kNandPageSize - column;
    if (chunk > len) {
      chunk = len;
    }

    esp_err_t ret = ESP_OK;
    if (column == 0 && chunk == kNandPageSize) {
      ret = nand_program_page(page, 0, src, kNandPageSize);
    } else {
      uint8_t page_buf[kNandPageSize];
      ret = nand_read_page(page, page_buf);
      if (ret != ESP_OK) {
        return ret;
      }
      memcpy(page_buf + column, src, chunk);
      ret = nand_program_page(page, 0, page_buf, kNandPageSize);
    }
    if (ret != ESP_OK) {
      return ret;
    }

    phys_addr += chunk;
    src += chunk;
    len -= chunk;

    if ((phys_addr % (kNandPageSize * 8)) == 0) {
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
  return ESP_OK;
}

static bool nand_is_block_bad(uint32_t block) {
  uint8_t marker = 0xFF;
  uint32_t page0 = block * kNandPagesPerBlock;
  if (nand_page_read(page0) != ESP_OK) {
    return true;
  }
  if (nand_read_cache(kNandSpareOffset, &marker, 1) != ESP_OK) {
    return true;
  }
  if (marker != 0xFF) {
    return true;
  }

  marker = 0xFF;
  uint32_t page1 = page0 + 1;
  if (nand_page_read(page1) != ESP_OK) {
    return true;
  }
  if (nand_read_cache(kNandSpareOffset, &marker, 1) != ESP_OK) {
    return true;
  }
  return marker != 0xFF;
}

static esp_err_t nand_build_block_map(void) {
  g_good_block_count = 0;
  for (uint32_t block = 0; block < kNandBlockCount; ++block) {
    if (!nand_is_block_bad(block)) {
      g_good_blocks[g_good_block_count++] = block;
    }
    if ((block % 16) == 0) {
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
  return g_good_block_count > 0 ? ESP_OK : ESP_FAIL;
}

static bool nand_map_addr(uint32_t addr, uint32_t *out_phys) {
  uint32_t block = addr / kNandBlockSize;
  uint32_t offset = addr % kNandBlockSize;
  if (block >= g_good_block_count) {
    return false;
  }
  *out_phys = g_good_blocks[block] * kNandBlockSize + offset;
  return true;
}

extern "C" void spiffs_api_lock(spiffs *fs) {
  auto *ctx = static_cast<spiffs_nand_ctx_t *>(fs->user_data);
  if (ctx && ctx->lock) {
    xSemaphoreTake(ctx->lock, portMAX_DELAY);
  }
}

extern "C" void spiffs_api_unlock(spiffs *fs) {
  auto *ctx = static_cast<spiffs_nand_ctx_t *>(fs->user_data);
  if (ctx && ctx->lock) {
    xSemaphoreGive(ctx->lock);
  }
}

static s32_t spiffs_nand_read(spiffs *fs, uint32_t addr, uint32_t size,
                              uint8_t *dst) {
  (void)fs;
  if (!nand_lock(portMAX_DELAY)) {
    return -1;
  }

  while (size > 0) {
    uint32_t phys_addr = 0;
    if (!nand_map_addr(addr, &phys_addr)) {
      nand_unlock();
      return -1;
    }

    uint32_t chunk = kNandBlockSize - (addr % kNandBlockSize);
    if (chunk > size) {
      chunk = size;
    }

    esp_err_t ret = nand_read_bytes(phys_addr, dst, chunk);
    if (ret != ESP_OK) {
      nand_unlock();
      return -1;
    }

    addr += chunk;
    dst += chunk;
    size -= chunk;
  }

  nand_unlock();
  return 0;
}

static s32_t spiffs_nand_write(spiffs *fs, uint32_t addr, uint32_t size,
                               uint8_t *src) {
  (void)fs;
  if (!nand_lock(portMAX_DELAY)) {
    return -1;
  }

  while (size > 0) {
    uint32_t phys_addr = 0;
    if (!nand_map_addr(addr, &phys_addr)) {
      nand_unlock();
      return -1;
    }

    uint32_t chunk = kNandBlockSize - (addr % kNandBlockSize);
    if (chunk > size) {
      chunk = size;
    }

    esp_err_t ret = nand_write_bytes(phys_addr, src, chunk);
    if (ret != ESP_OK) {
      nand_unlock();
      return -1;
    }

    addr += chunk;
    src += chunk;
    size -= chunk;
  }

  nand_unlock();
  return 0;
}

static s32_t spiffs_nand_erase(spiffs *fs, uint32_t addr, uint32_t size) {
  (void)fs;
  if (!nand_lock(portMAX_DELAY)) {
    return -1;
  }

  while (size > 0) {
    uint32_t block = addr / kNandBlockSize;
    if (block >= g_good_block_count) {
      nand_unlock();
      return -1;
    }

    uint32_t phys_page = g_good_blocks[block] * kNandPagesPerBlock;
    esp_err_t ret = nand_erase_block(phys_page);
    if (ret != ESP_OK) {
      nand_unlock();
      return -1;
    }

    addr += kNandBlockSize;
    if (size >= kNandBlockSize) {
      size -= kNandBlockSize;
    } else {
      size = 0;
    }

    vTaskDelay(pdMS_TO_TICKS(1));
  }

  nand_unlock();
  return 0;
}

static int log_storage_vprintf(const char *fmt, va_list args) {
  char line[256];
  va_list args_copy;
  va_copy(args_copy, args);
  int len = vsnprintf(line, sizeof(line), fmt, args_copy);
  va_end(args_copy);

  if (g_log_ready && g_log_ringbuf && len > 0 &&
      xTaskGetSchedulerState() == taskSCHEDULER_RUNNING &&
      !xPortInIsrContext()) {
    size_t write_len = (len >= (int)sizeof(line)) ? (sizeof(line) - 1) : len;
    xRingbufferSend(g_log_ringbuf, line, write_len, 0);
  }

  if (g_orig_vprintf) {
    va_list args_copy2;
    va_copy(args_copy2, args);
    int ret = g_orig_vprintf(fmt, args_copy2);
    va_end(args_copy2);
    return ret;
  }

  return len;
}

static void log_storage_task(void *param) {
  (void)param;
  uint32_t pending_bytes = 0;
  uint32_t last_flush_ms = 0;

  while (true) {
    if (!g_spiffs_mounted || g_log_file < 0 || !g_log_ringbuf) {
      vTaskDelay(pdMS_TO_TICKS(200));
      continue;
    }

    size_t item_size = 0;
    char *item = (char *)xRingbufferReceive(g_log_ringbuf, &item_size,
                                            pdMS_TO_TICKS(200));
    if (item) {
      if (g_log_file >= 0) {
        SPIFFS_write(&g_fs, g_log_file, item, item_size);
        pending_bytes += item_size;
      }
      vRingbufferReturnItem(g_log_ringbuf, item);
    }

    uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000);
    bool should_flush = pending_bytes >= kLogFlushBytes ||
                        (pending_bytes > 0 && (now_ms - last_flush_ms) > 2000);

    if (should_flush && g_log_file >= 0) {
      SPIFFS_fflush(&g_fs, g_log_file);
      pending_bytes = 0;
      last_flush_ms = now_ms;

      spiffs_stat stat = {};
      if (SPIFFS_fstat(&g_fs, g_log_file, &stat) == SPIFFS_OK &&
          stat.size > g_log_max_bytes) {
        SPIFFS_close(&g_fs, g_log_file);
        SPIFFS_remove(&g_fs, "log.txt");
        g_log_file = SPIFFS_open(&g_fs, "log.txt",
                                 SPIFFS_CREAT | SPIFFS_APPEND | SPIFFS_RDWR, 0);
      }
    }
  }
}

static esp_err_t log_storage_mount_spiffs(void) {
  g_spiffs_ctx.lock = xSemaphoreCreateMutex();
  if (!g_spiffs_ctx.lock) {
    return ESP_ERR_NO_MEM;
  }

  spiffs_config cfg = {};
  cfg.hal_read_f = spiffs_nand_read;
  cfg.hal_write_f = spiffs_nand_write;
  cfg.hal_erase_f = spiffs_nand_erase;
  cfg.phys_size = g_good_block_count * kNandBlockSize;
  cfg.phys_addr = 0;
  cfg.phys_erase_block = kNandBlockSize;
  cfg.log_block_size = kNandBlockSize;
  cfg.log_page_size = kNandPageSize;

  g_fs.cfg = cfg;
  g_fs.user_data = &g_spiffs_ctx;

  g_spiffs_work = (uint8_t *)heap_caps_malloc(kNandPageSize * 2, MALLOC_CAP_DEFAULT);
  g_spiffs_fds = (uint8_t *)heap_caps_malloc(4096, MALLOC_CAP_DEFAULT);
  g_spiffs_cache = (uint8_t *)heap_caps_malloc(kNandPageSize * 2 + 1024,
                                               MALLOC_CAP_DEFAULT);

  if (!g_spiffs_work || !g_spiffs_fds || !g_spiffs_cache) {
    return ESP_ERR_NO_MEM;
  }

  int res = SPIFFS_mount(&g_fs, &cfg, g_spiffs_work, g_spiffs_fds, 4096,
                         g_spiffs_cache, kNandPageSize * 2 + 1024, nullptr);
  if (res != SPIFFS_OK) {
    SPIFFS_format(&g_fs);
    res = SPIFFS_mount(&g_fs, &cfg, g_spiffs_work, g_spiffs_fds, 4096,
                       g_spiffs_cache, kNandPageSize * 2 + 1024, nullptr);
  }

  return (res == SPIFFS_OK) ? ESP_OK : ESP_FAIL;
}

static void log_storage_mount_task(void *param) {
  (void)param;
  vTaskDelay(pdMS_TO_TICKS(5000));

  ESP_LOGI(TAG, "=== NAND Flash Debug ===");
  ESP_LOGI(TAG, "SPI Host: SPI2_HOST, CS: GPIO%d, CLK: GPIO%d, MOSI: GPIO%d, MISO: GPIO%d",
           kNandPinCs, kNandPinSclk, kNandPinMosi, kNandPinMiso);
  ESP_LOGI(TAG, "SPI handle: %p", (void*)g_nand_spi);

  if (!g_nand_spi) {
    ESP_LOGE(TAG, "NAND SPI handle is NULL!");
    vTaskDelete(nullptr);
    return;
  }

  if (!nand_lock(pdMS_TO_TICKS(1000))) {
    ESP_LOGE(TAG, "NAND lock timeout");
    vTaskDelete(nullptr);
    return;
  }

  // Try read ID first (without reset) to see if chip responds at all
  ESP_LOGI(TAG, "Trying Read ID before reset...");
  uint8_t mfr = 0, dev1 = 0, dev2 = 0;
  esp_err_t ret = nand_read_id(&mfr, &dev1, &dev2);
  ESP_LOGI(TAG, "Pre-reset Read ID result: %s, ID: 0x%02X 0x%02X 0x%02X",
           esp_err_to_name(ret), mfr, dev1, dev2);

  ret = nand_reset();
  if (ret != ESP_OK) {
    nand_unlock();
    ESP_LOGE(TAG, "NAND reset failed: %s", esp_err_to_name(ret));
    vTaskDelete(nullptr);
    return;
  }

  mfr = 0; dev1 = 0; dev2 = 0;
  ret = nand_read_id(&mfr, &dev1, &dev2);
  nand_unlock();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "NAND read ID failed: %s", esp_err_to_name(ret));
    vTaskDelete(nullptr);
    return;
  }

  ESP_LOGI(TAG, "W25N512 ID: 0x%02X 0x%02X 0x%02X", mfr, dev1, dev2);

  if (!nand_lock(pdMS_TO_TICKS(1000))) {
    ESP_LOGE(TAG, "NAND lock timeout");
    vTaskDelete(nullptr);
    return;
  }
  ret = nand_build_block_map();
  nand_unlock();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "NAND block scan failed");
    vTaskDelete(nullptr);
    return;
  }

  ESP_LOGI(TAG, "NAND blocks: %u good / %u total", g_good_block_count,
           kNandBlockCount);
  uint32_t total_bytes = g_good_block_count * kNandBlockSize;
  if (total_bytes > kNandBlockSize) {
    uint32_t max_allowed = total_bytes - kNandBlockSize;
    if (g_log_max_bytes > max_allowed) {
      g_log_max_bytes = max_allowed;
    }
  }

  ret = log_storage_mount_spiffs();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "SPIFFS mount failed");
    vTaskDelete(nullptr);
    return;
  }

  g_log_file = SPIFFS_open(&g_fs, "log.txt",
                           SPIFFS_CREAT | SPIFFS_APPEND | SPIFFS_RDWR, 0);
  if (g_log_file < 0) {
    ESP_LOGE(TAG, "SPIFFS open log failed");
    vTaskDelete(nullptr);
    return;
  }

  g_spiffs_mounted = true;
  ESP_LOGI(TAG, "Log storage ready (SPIFFS on W25N512, %u bytes)",
           g_good_block_count * kNandBlockSize);

  vTaskDelete(nullptr);
}

esp_err_t log_storage_init(void) {
  if (g_log_ready) {
    return ESP_OK;
  }

  g_nand_lock = xSemaphoreCreateMutex();
  if (!g_nand_lock) {
    return ESP_ERR_NO_MEM;
  }

  spi_bus_config_t buscfg = {};
  buscfg.mosi_io_num = kNandPinMosi;
  buscfg.miso_io_num = kNandPinMiso;
  buscfg.sclk_io_num = kNandPinSclk;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.data4_io_num = -1;
  buscfg.data5_io_num = -1;
  buscfg.data6_io_num = -1;
  buscfg.data7_io_num = -1;
  buscfg.data_io_default_level = false;
  buscfg.max_transfer_sz = 4096;
  buscfg.flags = 0;
  buscfg.isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO;
  buscfg.intr_flags = 0;

  esp_err_t ret = spi_bus_initialize(kNandSpiHost, &buscfg, SPI_DMA_CH_AUTO);
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
    return ret;
  }

  spi_device_interface_config_t devcfg = {};
  devcfg.clock_speed_hz = kNandClockHz;
  devcfg.mode = 0;
  devcfg.spics_io_num = kNandPinCs;
  devcfg.queue_size = 1;
  devcfg.flags = SPI_DEVICE_HALFDUPLEX;

  ret = spi_bus_add_device(kNandSpiHost, &devcfg, &g_nand_spi);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "SPI NAND add device failed: %s", esp_err_to_name(ret));
    return ret;
  }

  g_log_ringbuf = xRingbufferCreate(kLogRingbufSize, RINGBUF_TYPE_BYTEBUF);
  if (!g_log_ringbuf) {
    return ESP_ERR_NO_MEM;
  }

  xTaskCreatePinnedToCore(log_storage_task, "LogTask", 4096, nullptr, 3,
                          &g_log_task, tskNO_AFFINITY);

  if (!g_orig_vprintf) {
    g_orig_vprintf = esp_log_set_vprintf(log_storage_vprintf);
  }
  g_log_ready = true;

  if (!g_mount_started) {
    g_mount_started = true;
    xTaskCreatePinnedToCore(log_storage_mount_task, "LogMount", 8192, nullptr, 1,
                            &g_mount_task, tskNO_AFFINITY);
  }
  return ESP_OK;
}

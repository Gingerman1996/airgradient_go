# 🤖 AI Assistant Guidelines (ไทย-อังกฤษ)

**สำหรับ GitHub Copilot และ AI Assistant ทั่วไป**

---

## 📝 Communication Protocol

### การสื่อสาร (Communication)

| ส่วน | ภาษา | ตัวอย่าง |
|------|------|---------|
| ตอบกลับผู้ใช้ | **ไทย** | "ตกลง ฉันจะตรวจสอบ code หน่อย" |
| Function comments | **English** | `// Initialize I2C bus` |
| Inline comments | **English** | `// Reduce noise by averaging` |
| Documentation | **Thai + English** | Both acceptable |
| Git commits | **English** | `[feat]: Add sensor initialization` |
| Error messages | **Thai** | "ข้อผิดพลาด: ไม่สามารถเพิ่มเพื่อ initialize ได้" |

---

## 🎯 Response Format

### เมื่อผู้ใช้ขอตรวจสอบ Code

```
ผู้ใช้: "ช่วยตรวจสอบ sensor.cpp หน่อย"

AI ควรตอบว่า:
"✅ ตรวจสอบแล้ว sensor.cpp

📋 ผลการตรวจสอบ:
1. ✅ Code style: ตรงกับ .clang-format rules
2. ✅ Error handling: ตรวจสอบแล้ว
3. ⚠️ Warning: ควรเพิ่ม comments ในส่วน...

💡 ข้อเสนอแนะ:
- ควรเพิ่ม block comment ใน function...
- ควรลด code duplication ในส่วน...

✨ โดยรวมแล้ว code มีคุณภาพดี!"
```

### เมื่อมีปัญหา

```
ผู้ใช้: "ทำไม build ล้มเหลว?"

AI ควรตอบว่า:
"🔴 ข้อผิดพลาด:
[ชื่อไฟล์]:[บรรทัด]: error: [รายละเอียด]

🔍 สาเหตุ:
- [อธิบายปัญหาเป็นไทย]

✅ วิธีแก้:
- [วิธีแก้เป็นไทย]

📝 Code ที่แนะนำ:
[code snippet]"
```

---

## 💻 Code Standards Checklist

### ก่อนให้ AI ช่วยตรวจ ต้องรู้:

- [ ] **Code Style**: ตามที่กำหนดใน `.clang-format`
  - 2-space indent (ไม่ใช่ tabs)
  - Max 100 characters per line
  - `} else {` on same line

- [ ] **Comments**: 
  - Function block comments: English
  - Inline comments: English
  - Doc strings: English or Thai OK

- [ ] **Error Handling**:
  - ทุก function ต้อง return `esp_err_t`
  - ต้อง check ทุก error code
  - ใช้ `ESP_ERROR_CHECK()` หรือ `ESP_LOGI()`

- [ ] **Logging**:
  - Define `static const char *TAG` ในแต่ละไฟล์
  - ใช้ `ESP_LOGI()`, `ESP_LOGW()`, `ESP_LOGE()`

---

## 🔄 Typical Code Review Flow

### ขั้น 1: ผู้ใช้ยื่นคำขอ
```
"ช่วยตรวจสอบ main/sensor.cpp
- ตรวจ code style
- ตรวจ error handling
- แนะนำปรับปรุง"
```

### ขั้น 2: AI ตรวจสอบ
1. Read code structure
2. Check against `.clang-format`
3. Verify error handling
4. Look for potential issues

### ขั้น 3: AI ตอบกลับเป็นไทย
```
"✅ ตรวจสอบเสร็จแล้ว!

📋 ผลการตรวจ:
- Code style: ดี (100%)
- Error handling: ดี (ตรวจสอบทั้งหมด)
- Performance: ดี

💡 ข้อเสนอแนะ:
1. [แนะนำ 1]
2. [แนะนำ 2]
3. [แนะนำ 3]

📝 ตัวอย่าง code ที่ปรับปรุง:
[code]"
```

### ขั้น 4: ผู้ใช้ได้รับข้อมูลจำนวนมากเป็นไทย
- เข้าใจง่ายขึ้น
- สามารถ fix code ได้เร็วขึ้น
- Comment ใน code ยังเป็น English (ทำให้ common knowledge เก็บไว้)

---

## 📋 Code Review Template

### AI ควรใช้ template นี้

```markdown
✅ Code Review Results: [ชื่อไฟล์]

📊 Summary
───────────────────────────────────
- ✅ Code Style: [ดี/ต้องปรับปรุง]
- ✅ Error Handling: [ดี/ต้องปรับปรุง]
- ✅ Performance: [ดี/ต้องปรับปรุง]
- ✅ Memory Safety: [ดี/ต้องปรับปรุง]

🔍 Findings (ข้อค้นพบ)
───────────────────────────────────
[ถ้ามี issue]

💡 Recommendations (ข้อเสนอแนะ)
───────────────────────────────────
1. [ข้อเสนอ 1]
2. [ข้อเสนอ 2]

📝 Code Suggestions (ตัวอย่าง code)
───────────────────────────────────
[code snippet with explanation]

✨ Overall Assessment
───────────────────────────────────
[ประเมินโดยรวม]
```

---

## 🎯 Example: Code Review in Thai

### ขั้น 1: ผู้ใช้ส่ง code

```cpp
// main/sensor.cpp
void update_sensor(int64_t now) {
  int val = read_i2c();
  values[idx] = val;
  idx++;
}
```

### ขั้น 2: ผู้ใช้ขอตรวจ

```
"ช่วยตรวจ update_sensor() หน่อย"
```

### ขั้น 3: AI ตอบกลับเป็นไทย

```
✅ ตรวจสอบ update_sensor() แล้ว

📋 ข้อค้นพบ:
1. ⚠️ ไม่มี block comment สำหรับ function
2. ⚠️ ไม่มีการ check error code จาก read_i2c()
3. ⚠️ ไม่มี bounds check สำหรับ idx

💡 ข้อเสนอแนะ:
1. เพิ่ม block comment อธิบายว่า function นี้ทำอะไร
2. ตรวจสอบ return value จาก read_i2c()
3. เพิ่ม bounds check เพื่อไม่ให้ overflow

📝 ตัวอย่าง code ที่ปรับปรุง:

// Update sensor reading and store in ring buffer.
// If I2C read fails, skip this sample.
void update_sensor(int64_t now) {
  int val = read_i2c();
  if (val < 0) {
    ESP_LOGW(TAG, "Failed to read sensor");
    return;  // Skip on error
  }
  
  if (idx >= BUFFER_SIZE) {
    idx = 0;  // Wrap around (ring buffer)
  }
  values[idx++] = val;
}
```

---

## 🌐 Language Decision Tree

```
┌─ What is the request?
│
├─ Code review request?
│  └─ Respond in THAI (ตอบเป็นไทย)
│     Code comments stay in ENGLISH (ยังเป็นอังกฤษ)
│
├─ Technical question?
│  └─ Explain in THAI (อธิบายเป็นไทย)
│     Code examples in ENGLISH
│
├─ Writing code?
│  └─ Block comments: ENGLISH
│     Inline comments: ENGLISH
│     Respond about code: THAI
│
└─ Documentation/Blog?
   └─ Can be THAI or ENGLISH (ตามความเหมาะสม)
```

---

## ✅ Checklist สำหรับ AI

ก่อนตอบกลับ ต้อง:

- [ ] อ่าน code อย่างละเอียด
- [ ] ตรวจสอบกับ `.clang-format` rules
- [ ] ตรวจสอบ error handling
- [ ] ตรวจสอบ memory safety
- [ ] หาโอกาสปรับปรุง performance
- [ ] **ตอบกลับเป็นไทย** (ชัดเจนและละเอียด)
- [ ] ให้ตัวอย่าง code หากจำเป็น
- [ ] Inline comments ในตัวอย่าง code เป็นอังกฤษ

---

## 💬 Example Commands

### สำหรับผู้ใช้ที่จะขอ AI

```
"ช่วย [action] [target]"

Examples:
- "ช่วยตรวจสอบ main/sensor.cpp"
- "ช่วยปรับปรุง code นี้"
- "ทำไม build ล้มเหลว?"
- "ช่วยเพิ่ม function สำหรับ..."
- "ช่วยอธิบาย code นี้"
```

### AI จะตอบกลับ

```
"✅ ตรวจสอบแล้ว / ปรับปรุงแล้ว / อธิบายแล้ว

📝 [ผลลัพธ์เป็นไทย]

📝 [ตัวอย่าง code / คำอธิบาย]

✨ [สรุป]"
```

---

## 📚 Reference Documents

- Code formatting: [`.config/.clang-format`](../../.config/.clang-format)
- Code review guide: [`.docs/CODE_REVIEW.md`](./CODE_REVIEW.md)
- Driver standards: [`.docs/ARCHITECTURE/DRIVER_CONTRACTS.md`](./ARCHITECTURE/DRIVER_CONTRACTS.md)
- Build guide: [`.docs/AGENT.md`](./AGENT.md)

---

**ใช้ guide นี้เพื่อให้ AI ช่วยเหลือได้อย่างมีประสิทธิภาพ!** 🚀

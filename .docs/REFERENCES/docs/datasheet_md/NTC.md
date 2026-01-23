```
外形尺寸 ShapeandDimensions
```
```
 尺寸：见图 1 和表 1
 PCB焊盘：见图 2 和表 1
```
```
 Dimensions:SeeFig. 1 andTable 1.
 RecommendedPCBpatternforreflowsoldering:SeeFig. 2 andTable 1
```
```
图 1 Fig. 1 图 2 Fig. 2
```
```
表 1 （ Table 1 ） 单位unit:inch[mm]
类别Type L W T a A B C
0805
[ 2012 ]
```
```
0. 079 ± 0. 008
[ 2. 0 ± 0. 2 ]
```
```
0. 049 ± 0. 008
[ 1. 25 ± 0. 2 ]
```
```
0. 033 ± 0. 008
[ 0. 85 ± 0. 2 ]
```
```
0. 020 ± 0. 012
[ 0. 5 ± 0. 3 ]
[ 1. 0 - 1. 1 ] [ 0. 6 - 0. 7 ] [ 1. 0 - 1. 2 ]
```
```
电气特性 ElectricalCharacteristics
```
```
型号
PartNo
```
```
电阻值
Resistance
( 25 °C)
(kΩ)
```
```
B常数
BConstant
( 25 / 50 °C)
（K）
```
```
B常数
BConstant
( 25 / 85 °C)
（K）
```
```
允许工作电流
Permissible
OperatingCurrent
( 25 °C)
（mA）
```
```
耗散系数
Dissipation
Factor
(mW/°C)
```
```
热时间常数
Thermal
Time
Constant
(s)
```
```
额定功率
RatedElectric
Power( 25 °C)
(mW)
```
```
工作温度
Operating
ambient
temperature
(°C)
```
10 ± 1 % 3950 ± 1 % (^39870). 44 2. (^0) ＜ 5 100 - (^40) ～+ 125

# KNTC0805/10KF

KNTC0805/10KF


### 检验和测试程序

### 测试条件

### 如无特别规定，检验和测试的标准大气环境条件如下：

```
a. 环境温度： 20 ± 15 °C；
b.相对湿度： 65 ± 20 %；
c. 气压： 86 kPa~ 106 kPa
如果对测试结果有异议，则在下述条件下测试：
a. 环境温度： 25 ± 2 °C；
b.相对湿度： 65 ± 5 %
c. 气压： 86 kPa~ 106 kPa
```
### 检查设备

### 外观检查： 20 倍放大镜；

### 阻值检查：热敏电阻测试仪

```
TestandMeasurementProcedures
TestConditions
Unlessotherwisespecified,thestandardatmospheric
conditionsformeasurement/testas:
a.AmbientTemperature: 20 ± 15 °C
b.RelativeHumidity: 65 ± 20 %
c.AirPressure: 86 kPato 106 kPa
Ifanydoubtontheresults,measurements/testsshould
bemadewithinthefollowinglimits:
a.AmbientTemperature: 25 ± 2 °C
b.RelativeHumidity: 65 ± 5 %
c.AirPressure: 86 kPato 106 kPa
```
```
InspectionEquipment
VisualExamination: 20 ×magnifier
Resistancevaluetest:Thermistorresistancetester
```
```
电性测试 ElectricalTest
```
序号 **No.** 项目 **Items** 测试方法及备注 **TestMethodsandRemarks**

```
1
```
```
25 °C零功率电阻值
NominalZero-PowerResistanceat
25 °C(R 25 )
```
```
环境温度Ambienttemperature： 25 ± 0. 05 °C
测试功率Measuringelectricpower：≤^0.^1 mW
```
```
2
```
```
B值常数
NominalBConstant
```
```
分别在环境温度 25 ± 0. 05 °C， 50 ± 0. 05 °C或 85 ± 0. 05 °C下测量电阻值。
Measuretheresistanceattheambient temperatureof 25 ± 0. 05 °C, 50 ± 0. 05 °Cor
85 ± 0. 05 °C.
B( 25 - 50 °C)= 1 lnRT^2255 −−l 1 nRT^5500 B( 25 - 85 °C)= 1 lnRT^2255 −−l 1 nRT^8855
T：绝对温度（K）Absolutetemperature(K)
```
(^3) Therm热al时Ti间m常eC数onstant
在零功率条件下，当热敏电阻的环境温度发生急剧变化时，热敏电阻元件产生
最初温度T 0 与最终温度T 1 两者温度差的 63. 2 %的温度变化所需要的时间，通
常以秒(S)表示。
Thetotaltimeforthetemperatureofthethermistortochangeby 63. 2 %ofthe
differencefromambienttemperatureT 0 (°C)toT 1 (°C)bythedrasticchangeofthe
powerappliedtothermistorfromNon-zeroPowertoZero-Powerstate,normally
expressedinsecond(S).


```
4
耗散系数
DissipationFactor
```
```
在一定环境温度下，NTC热敏电阻通过自身发热使其温度升高 1 °C时所需要的
功率，通常以mW/°C表示。可由下面公式计算：
TherequiredpowerwhichmakestheNTCthermistorbodytemperatureraise 1 °C
throughself-heated,normallyexpressedinmilliwattsperdegreeCelsius(mW/°C).It
canbecalculatedbythefollowingformula:
```
```
δ=T−WT 0
```
```
5
额定功率
RatedPower
```
```
在环境温度^25 °C下因自身发热使表面温度升高^100 °C所需要的功率。
The necessary electric power makes thermistor’s temperature rise 100 °C by
self-heatingatambienttemperature 25 °C.
```
```
6
允许工作电流
Permissibleoperatingcurrent
```
```
在静止空气中通过自身发热使其升温为 1 °C的电流。
ThecurrentthatkeepbodytemperatureofchipNTConthePCboardinstillair
rising 1 °Cbyself-heating.
```
```
信赖性试验 ReliabilityTest
```
```
项目
Items
```
```
测试标准
Standard
```
```
测试方法及备注
TestMethodsandRemarks
```
```
要求
Requirements
```
端头附着力
Terminal
Strength

```
IEC 60068 - 2 - 21
```
```
将晶片焊接在测试基板上（如右图所示的环氧玻璃布板），按箭头
所示方向施加作用力；
Solderthechiptothetestingjig(glassepoxyboardshownintheright)
usingeutecticsolder.Thenapplyaforceinthedirectionofthearrow.
```
```
尺寸Size F 保持时间Duration
0201 , 0402 , 0603 5 N
10 ± 1 s
0805 10 N
```
```
端电极无脱落且瓷体无损伤。
Noremovalorsplitofthetermination
orotherdefectsshalloccur.
```
```
抗弯强度
Resistance
toFlexure
```
```
IEC 60068 - 2 - 21
```
```
将晶片焊接在测试基板上（如右图所示的环氧玻璃布板），按下图
箭头所示方向施加作用力；
Solderthechiptothetestjig(glassepoxyboardshownintheright)
usingaeutecticsolder.Thenapplyaforceinthedirectionshownas
follow;
```
```
尺寸
Size
```
```
弯曲变形量
Flexure
```
```
施压速度
Pressurizing
Speed
```
```
保持时间
Duration
```
```
0201 , 1 mm
＜ 0. 5 mm/s 10 ± 1 s
0402 , 0603 , 0805 2 mm
```
```
1 无外观损伤。
Novisibledamage.
2 ∣∆R 25 /R 25 ∣≤ 5 %
```
```
单位unit：mm
类型Type a b c
0201 0. 25 0. 3 0. 3
0402 0. 4 1. 5 0. 5
0603 1. 0 3. 0 1. 2
0805 1. 2 4. 0 1. 65
```

```
振动
Vibration
IEC 60068 - 2 - 80
```
```
1 将晶片焊接在测试基板上（如右图所示的环氧玻璃布板）；
Solderthechiptothetestingjig(glassepoxyboardshowninthe
left)usingeutecticsolder.
2 晶片以全振幅为 1. 5 mm进行振动，频率范围为 10 Hz～ 55 Hz；
Thechipshallbesubjectedtoasimpleharmonicmotionhaving
totalamplitudeof 1. 5 mm,thefrequencybeingvarieduniformly
betweentheapproximatelimitsof 10 and 55 Hz.
3 振动频率按 10 Hz→ 55 Hz→ 10 Hz循环，周期为 1 分钟，在空间
三个互相垂直的方向上各振动 2 小时（共 6 小时）。
Thefrequencyrangesfrom 10 to 55 Hzandreturnto 10 Hzshall
betraversedinapproximately 1 minute.Thismotionshall be
appliedforaperiodof 2 hoursineach 3 mutuallyperpendicular
directions(totalof 6 hours).
```
```
无外观损伤。
Novisibledamage.
```
```
坠落
Dropping
IEC 60068 - 2 - 32
从 1 m的高度让晶片自由坠落至水泥地面 10 次。
Dropachip 10 timesonaconcretefloorfromaheightof 1 meter.
```
```
无外观损伤。
Novisibledamage.
```
可焊性
Solderability
IEC 60068 - 2 - 58

```
1 焊接温度Soldertemperature: 245 ± 5 °C.
2 浸渍时间Duration: 3 ± 0. 3 s.
3 焊锡成分Solder:Sn/ 3. 0 Ag/ 0. 5 Cu.
```
(^4) 助焊剂Flux:（重量比） 25 %松香和 75 %酒精
25 %Resinand 75 %ethanolinweight.
1 无外观损伤；
Novisibledamage.
2 元件端电极的焊锡覆盖率不小于
95 %。
Wetting shall exceed 95 %
coverage.
耐焊性
Resistance
toSoldering
Heat
IEC 60068 - 2 - 58
1 焊接温度Soldertemperature: 260 ± 5 °C.
(^2) 浸渍时间Duration: 10 ± 1 s.
3 焊锡成分Solder:Sn/ 3. 0 Ag/ 0. 5 Cu.
4 助焊剂Flux:（重量比） 25 %松香和 75 %酒精
25 %Resinand 75 %ethanolinweight.
5 试验后标准条件下放置 1 ~ 2 小时后测量。
Thechipshallbestabilizedatnormalconditionfor 1 ~ 2 hours
beforemeasuring.
1 无外观损伤；
Novisibledamage.
2 ∣∆R 25 /R 25 ∣≤ 5 %
3 ∣∆B/B∣≤ 2 %
温度周期
Temperature
cycling
IEC 60068 - 2 - 14
(^1) 无负载于下表所示的环境条件下重复 (^5) 次。
5 cyclesoffollowingsequencewithoutloading.
步骤Step 温度Temperature 时间Time
1 - 40 ± 5 °C 30 ± 3 min
2 25 ± 2 °C 5 ± 3 min
3 125 ± 2 °C 30 ± 3 min
4 25 ± 2 °C 5 ± 3 min
2 试验后标准条件下放置 1 ~ 2 小时后测量。
Thechipshallbestabilizedatnormalconditionfor 1 ~ 2 hours
beforemeasuring.
1 无外观损伤；
Novisibledamage.
2 ∣∆R 25 /R 25 ∣≤ 3 %
3 ∣∆B/B∣≤ 2 %
高温存放
Resistance
todryheat
IEC 60068 - 2 - 2
1 在 125 ± 5 °C空气中，无负载放置 1000 ± 24 小时。
125 ± 5 °Cinair,for 1000 ± 24 hourswithoutloading.
(^2) 试验后标准条件下放置 1 ~ (^2) 小时后测量。
Thechipshallbestabilizedatnormalconditionfor 1 ~ 2 hours
beforemeasuring.
1 无外观损伤；
Novisibledamage.
2 ∣∆R 25 /R 25 ∣≤ 5 %
3 ∣∆B/B∣≤ 2 %


```
编带 Taping
```
```
类型Type 0201 0402 0603 0805
编带厚度
Tapethickness(mm)^0.^5 ±^0.^150.^5 ±^0.^150.^8 ±^0.^150.^85 ±^0.^2
编带材质
Tapematerial 纸带 PaperTape
每盘数量
QuantityperReel^15 K^10 K^4 K^4 K
```
```
( 1 )编带图TapingDrawings
```
```
低温存放
Resistance
tocold
```
```
IEC 60068 - 2 - 1
```
```
1 在- 40 ± 3 °C空气中，无负载放置 1000 ± 24 小时。
```
- 40 ± 3 °Cinair,for 1000 ± 24 hourswithoutloading.
2 试验后标准条件下放置 1 ~ 2 小时后测量。
Thechipshallbestabilizedatnormalconditionfor 1 ~ 2 hours
beforemeasuring.

```
1 无外观损伤；
Novisibledamage.
2 ∣∆R 25 /R 25 ∣≤ 5 %
3 ∣∆B/B∣≤ 2 %
```
湿热存放
Resistance
todampheat

```
IEC 60068 - 2 - 78
```
```
1 在 40 ± 2 °C，相对湿度 90 ~ 95 %空气中，无负载放置 1000 ± 24 小
时。
40 ± 2 °C, 90 ~ 95 %RHinair,for 1000 ± 24 hourswithoutloading.
2 试验后标准条件下放置 1 ~ 2 小时后测量。
Thechipshallbestabilizedatnormalconditionfor 1 ~ 2 hours
beforemeasuring.
```
```
1 无外观损伤；
Novisibledamage.
2 ∣∆R 25 /R 25 ∣≤ 3 %
3 ∣∆B/B∣≤ 2 %
```
```
高温负荷
Resistance
tohigh
temperature
load
```
```
IEC 60539 - 1
5. 25. 4
```
(^1) 在 85 ± 2 °C空气中，施加允许工作电流 1000 ± 48 小时。
85 ± 2 °Cinairwithpermissiveoperatingcurrentfor 1000 ± 48 hours
2 试验后标准条件下放置 1 ~ 2 小时后测量。
Thechipshallbestabilizedatnormalconditionfor 1 ~ 2 hours
beforemeasuring.
1 无外观损伤；
Novisibledamage.
(^2) ∣∆R 25 /R 25 ∣≤ 5 %
3 ∣∆B/B∣≤ 2 %


( 2 )纸带尺寸PaperTapeDimensions (单位Unit：mm)

```
0201 系列 0402 系列
```
### 0603 系列 0805 系列

( 3 )卷盘尺寸ReelDimensions(单位Unit：mm)

```
A B
```
4. 3 ± 0. (^29). 0 ± 1. 5
58 ± 2. 0
178 ± 2. 0
4. 0 ± 0. 1 5.^0 ±^0.^1
3. 0 ± 0. 1
2. 45 ± 0. 2
13. 5 ± 0. 2


### 储存

###  储存条件

```
a.储存温度：- 10 °C～ 40 °C
b.相对湿度：≤ 75 %RH
c.避免接触粉尘、腐蚀性气氛和阳光
```
 储存期限：产品交付后 **6** 个月

```
Storage
 StorageConditions
a.StorageTemperature:- 10 °C～ 40 °C
b.RelativeHumidity:≦ 75 %RH
c.Keepawayfromcorrosiveatmosphereandsunlight.
```
```
 PeriodofStorage: 6 Monthsafterdelivery
```
### 注意事项

###  热敏电阻不可在以下条件下工作或储存：

### ( 1 )腐蚀性气体或还原性气体

### (氯气、硫化氢气体、氨气、硫酸气体、一氧化氮等)。

### ( 2 )挥发性或易燃性气体

### ( 3 )多尘条件

### ( 4 )高压或低压条件

### ( 5 )潮湿场所

### ( 6 )存在盐水、油、化学液体或有机溶剂的场所

### ( 7 )强烈振动

### ( 8 )存在类似有害条件的其他场所

###  热敏电阻的陶瓷属于易碎材料，使用时不可

### 施加过大压力或冲击。

###  热敏电阻不可在超过目录规定的温度范围情

### 况下工作。

```
Notes ＆ Warnings
 The thermistorsshallnotbe operatedand
storedunderthefollowingenvironmentalcondition:
( 1 )Corrosiveordeoxidizedatmospheres
(suchaschlorine,sulfuratedhydrogen,ammonia,sulfuric
acid,nitricoxideandsoon)
( 2 )Volatileorinflammableatmospheres
( 3 )Dustycondition
( 4 )Excessivelyhighorlowpressurecondition
( 5 )Humidsite
( 6 ) Places with brine, oil, chemical liquid or organic
solvent
( 7 )Intensevibration
( 8 )Placeswithanalogouslydeleteriousconditions
 Theceramicbodyofthe thermistorsisfragile,
noexcessivepressureorimpactshallbeexertedonit.
```
```
 The thermistorsshallnotbeoperatedbeyond
the specified “Operating Temperature Range” in the
catalog.
```

### 建议焊接条件

###  回流焊

```
温升 1 ~ 2 °C/sec.
预热： 150 ~ 170 °C/ 90 ± 30 sec.
大于 240 °C时间： 20 ~ 40 sec
峰值温度：最高 260 °C/ 10 sec.
焊锡： 96. 5 Sn/ 3. 0 Ag/ 0. 5 Cu
回流焊：最多 2 次
```
```
10 RecommendedSolderingTechnologies
 Re-flowingProfile
1 ~ 2 °C/sec.Ramp
Pre-heating: 150 ~ 170 °C/ 90 ± 30 sec.
Timeabove 240 °C: 20 ~ 40 sec.
Peaktemperature: 260 °CMax./ 10 sec.
Solderpaste: 96. 5 Sn/ 3. 0 Ag/ 0. 5 Cu
Max. 2 timesforre-flowing
```
###  手工焊

### 烙铁功率：最大 20 W

```
预热： 150 °C/ 60 sec.
烙铁头温度：最高 280 °C
焊接时间：最多 3 sec.
焊锡： 96. 5 Sn/ 3. 0 Ag/ 0. 5 Cu
手工焊：最多 1 次
```
### [注：不要使烙铁头接触到端头]

```
 IronSolderingProfile
Ironsolderingpower:Max. 20 W
Pre-heating: 150 °C/ 60 sec.
SolderingTiptemperature: 280 °CMax.
Solderingtime: 3 secMax.
Solderpaste: 96. 5 Sn/ 3. 0 Ag/ 0. 5 Cu
Max. 1 timesforironsoldering
```
```
[Note:Takecarenottoapplythetipof
thesolderingirontotheterminalelectrodes.]
```

## R-T 表 R-Ttable

### 温度

```
Temp.(°C)
```
### R最小值

```
R_Min(Kohm)
```
### R中心值

```
R_Cent(Kohm)
```
### R最大值

```
R_Max(Kohm)
```
### 阻值公差

```
ResTOL.
```
### 温度公差


### 温度

Temp.(°C)

### R最小值

```
R_Min(Kohm)
```
### R中心值

```
R_Cent(Kohm)
```
### R最大值

```
R_Max(Kohm)
```
### 阻值公差

```
ResTOL.
```
### 温度公差


### 温度

Temp.(°C)

### R最小值

R_Min(Kohm)

### R中心值

R_Cent(Kohm)

### R最大值

R_Max(Kohm)

### 阻值公差

ResTOL.

### 温度公差

- - 40 329 927 345 275 361 300 4 64 % Temp.TOL.(°C)
- - 39 308 651 322 791 337 545 4 57 %
- - 38 288 892 301 925 315 514 4 50 %
- - 37 270 532 282 549 295 071 4 43 %
- - 36 253 464 264 549 276 091 4 36 %
- - 35 237 587 247 816 258 459 4 29 %
- - 34 222 812 232 254 242 072 4 23 %
- - 33 209 055 217 774 226 833 4 16 %
- - 32 196 239 204 292 212 655 4 09 %
- - 31 184 293 191 735 199 457 4 03 %
- - 30 173 153 180 032 187 165 3 96 %
- - 29 162 760 169 120 175 711 3 90 %
- - 28 153 059 158 941 165 033 3 83 %
- - 27 144 000 149 441 155 073 3 77 %
- - 26 135 535 140 571 145 779 3 71 %
- - 25 127 622 132 284 137 102 3 64 %
- - 24 120 207 124 522 128 979 3 58 %
- - 23 113 270 117 266 121 391 3 52 %
- - 22 106 779 110 480 114 298 3 46 %
- - 21 100 701 104 130 107 664 3 39 %
- - 20 95 008 98 185 101 459 3 33 %
- - 19 89 674 92 618 95 650 3 27 %
- - 18 84 672 87 402 90 211 3 21 %
- - 17 79 982 82 513 85 115 3 15 %
- - 16 75 580 77 927 80 339 3 10 %
- - 15 71 449 73 626 75 861 3 04 %
- - 14 67 569 69 588 71 661 2 98 %
- - 13 63 924 65 797 67 719 2 92 %
- - 12 60 498 62 237 64 019 2 86 %
- - 11 57 277 58 890 60 543 2 81 %
- - 10 54 247 55 744 57 278 2 75 %
   - - 9 51 396 52 786 54 208 2 69 %
   - - 8 48 712 50 002 51 322 2 64 %
   - - 7 46 184 47 382 48 606 2 58 %
   - - 6 43 803 44 916 46 051 2 53 %
   - - 5 41 559 42 592 43 646 2 47 %
   - - 4 39 441 40 400 41 377 2 42 %
   - - 3 37 443 38 333 39 240 2 37 %
   - - 2 35 559 36 385 37 227 2 31 %
   - - 1 33 781 34 548 35 328 2 26 %
      - 0 32 102 32 814 33 538 2 21 %
      - 1 30 518 31 179 31 851 2 16 %
   - 2 29 022 29 636 30 259 2 10 % Temp.TOL.(°C)
   - 3 27 608 28 178 28 756 2 05 %
   - 4 26 271 26 800 27 336 2 00 %
   - 5 25 007 25 497 25 994 1 95 %
   - 6 23 808 24 263 24 724 1 90 %
   - 7 22 674 23 096 23 523 1 85 %
   - 8 21 601 21 992 22 387 1 80 %
   - 9 20 584 20 947 21 313 1 75 %
- 10 19 622 19 958 20 297 1 70 %
- 11 18 711 19 022 19 336 1 65 %
- 12 17 847 18 135 18 425 1 60 %
- 13 17 028 17 294 17 563 1 55 %
- 14 16 251 16 498 16 746 1 51 %
- 15 15 514 15 742 15 972 1 46 %
- 16 14 814 15 025 15 237 1 41 %
- 17 14 150 14 345 14 541 1 37 %
- 18 13 519 13 699 13 880 1 32 %
- 19 12 921 13 086 13 253 1 27 %
- 20 12 351 12 504 12 657 1 23 %
- 21 11 811 11 951 12 092 1 18 %
- 22 11 296 11 426 11 555 1 13 %
- 23 10 808 10 926 11 045 1 09 %
- 24 10 342 10 452 10 561 1 04 %
- 25 9 900 10 000 10 100 1 00 %
- 26 9 471 9 570 9 670 1 04 %
- 27 9 062 9 162 9 261 1 09 %
- 28 8 673 8 773 8 872 1 13 %
- 29 8 304 8 402 8 501 1 18 %
- 30 7 952 8 049 8 147 1 22 %
- 31 7 616 7 713 7 811 1 26 %
- 32 7 297 7 393 7 490 1 31 %
- 33 6 993 7 088 7 184 1 35 %
- 34 6 703 6 797 6 892 1 39 %
- 35 6 427 6 520 6 613 1 43 %
- 36 6 164 6 255 6 348 1 47 %
- 37 5 913 6 003 6 094 1 52 %
- 38 5 673 5 762 5 852 1 56 %
- 39 5 445 5 532 5 621 1 60 %
- 40 5 226 5 313 5 400 1 64 %
- 41 5 018 5 103 5 189 1 68 %
- 42 4 819 4 903 4 987 1 72 %
- 43 4 629 4 711 4 795 1 76 %
- 44 4 448 4 529 4 610 1 80 %
- 45 4 274 4 354 4 434 1 84 %
- 46 4 109 4 187 4 266 1 88 %
- 47 3 951 4 027 4 104 1 92 % Temp.TOL.(°C)
- 48 3 799 3 874 3 950 1 96 %
- 49 3 655 3 728 3 803 2 00 %
- 50 3 516 3 588 3 661 2 04 %
- 51 3 384 3 454 3 526 2 08 %
- 52 3 257 3 326 3 396 2 12 %
- 53 3 135 3 203 3 272 2 16 %
- 54 3 019 3 086 3 153 2 19 %
- 55 2 908 2 973 3 039 2 23 %
- 56 2 801 2 865 2 930 2 27 %
- 57 2 699 2 761 2 825 2 31 %
- 58 2 601 2 662 2 724 2 35 %
- 59 2 507 2 567 2 628 2 38 %
- 60 2 417 2 476 2 535 2 42 %
- 61 2 331 2 388 2 447 2 46 %
- 62 2 248 2 304 2 362 2 49 %
- 63 2 169 2 224 2 280 2 53 %
- 64 2 092 2 146 2 201 2 57 %
- 65 2 019 2 072 2 126 2 60 %
- 66 1 949 2 001 2 053 2 64 %
- 67 1 882 1 932 1 984 2 67 %
- 68 1 817 1 866 1 917 2 71 %
- 69 1 754 1 803 1 852 2 75 %
- 70 1 695 1 742 1 790 2 78 %
- 71 1 637 1 684 1 731 2 82 %
- 72 1 582 1 628 1 674 2 85 %
- 73 1 529 1 574 1 619 2 88 %
- 74 1 479 1 522 1 566 2 92 %
- 75 1 430 1 472 1 516 2 95 %
- 76 1 383 1 424 1 467 2 99 %
- 77 1 337 1 378 1 419 3 02 %
- 78 1 294 1 333 1 374 3 06 %
- 79 1 251 1 290 1 330 3 09 %
- 80 1 211 1 249 1 288 3 12 %
- 81 1 172 1 209 1 247 3 16 %
- 82 1 135 1 171 1 208 3 19 %
- 83 1 099 1 134 1 171 3 22 %
- 84 1 064 1 099 1 135 3 26 %
- 85 1 031 1 065 1 100 3 29 %
- 86 0 998 1 032 1 066 3 32 %
- 87 0 967 1 000 1 033 3 35 %
- 88 0 937 0 969 1 002 3 38 %
- 89 0 908 0 940 0 972 3 42 %
- 90 0 881 0 911 0 943 3 45 %
- 91 0 854 0 884 0 914 3 48 %


### 温度

Temp.(°C)

### R最小值

```
R_Min(Kohm)
```
### R中心值

```
R_Cent(Kohm)
```
### R最大值

```
R_Max(Kohm)
```
### 阻值公差

```
ResTOL.
```
### 温度公差

```
Temp.TOL.(°C)
92 0. 828 0. 857 0. 887 3. 51 % 1. 17
93 0. 803 0. 831 0. 861 3. 54 % 1. 19
94 0. 779 0. 807 0. 835 3. 57 % 1. 21
95 0. 755 0. 783 0. 811 3. 61 % 1. 22
96 0. 733 0. 760 0. 787 3. 64 % 1. 24
97 0. 711 0. 738 0. 765 3. 67 % 1. 26
98 0. 691 0. 716 0. 743 3. 70 % 1. 27
99 0. 670 0. 695 0. 721 3. 73 % 1. 29
100 0. 651 0. 675 0. 701 3. 76 % 1. 31
101 0. 632 0. 656 0. 681 3. 79 % 1. 33
102 0. 614 0. 637 0. 662 3. 82 % 1. 34
103 0. 596 0. 619 0. 643 3. 85 % 1. 36
104 0. 579 0. 602 0. 625 3. 88 % 1. 38
105 0. 563 0. 585 0. 608 3. 91 % 1. 40
106 0. 547 0. 569 0. 591 3. 94 % 1. 41
107 0. 532 0. 553 0. 575 3. 97 % 1. 43
108 0. 517 0. 538 0. 559 4. 00 % 1. 45
109 0. 502 0. 523 0. 544 4. 03 % 1. 47
110 0. 489 0. 508 0. 529 4. 05 % 1. 49
111 0. 475 0. 495 0. 515 4. 08 % 1. 50
112 0. 462 0. 481 0. 501 4. 11 % 1. 52
113 0. 449 0. 468 0. 487 4. 14 % 1. 54
114 0. 437 0. 456 0. 474 4. 17 % 1. 56
115 0. 425 0. 443 0. 462 4. 20 % 1. 58
116 0. 414 0. 432 0. 450 4. 22 % 1. 60
117 0. 403 0. 420 0. 438 4. 25 % 1. 62
118 0. 392 0. 409 0. 427 4. 28 % 1. 63
119 0. 382 0. 399 0. 416 4. 31 % 1. 65
120 0. 372 0. 388 0. 405 4. 34 % 1. 67
121 0. 362 0. 378 0. 395 4. 36 % 1. 69
122 0. 353 0. 368 0. 385 4. 39 % 1. 71
123 0. 344 0. 359 0. 375 4. 42 % 1. 73
124 0. 335 0. 350 0. 365 4. 44 % 1. 75
125 0. 326 0. 341 0. 356 4. 47 % 1. 77
```


# Raspberry Pi FDD
Raspberry Pi を NEC PC-8031-2W/PC-80S31 のようなインテリジェントタイプのFDDユニットにします。
d88形式の2D FDDイメージに対応しています。2ドライブです。  
注：FDDユニット側のCPUやROMには対応していません。

## 必要なハードウェア
- Raspberry Pi 4 (他のRaspberry Piでも動くと思いますが、試していません。)
- PC-8001/PC-8801/PC-9801シリーズの320KBインテリジェントタイプFDD用コネクタから信号線を取り出してRaspberry Piの各GPIOピンに繋ぐケーブル。

## ピンアサイン
```
FDD Port(5V)           Raspberry Pi(3.3V)
Pin 1- 8 (PB0-PB7) --> GPIO4-11 (*)3.3Vに変換して繋いで下さい
Pin19-26 (PA0-PA7) <-- GPIO12-19
Pin27-30 (PC4-7)   --> GPIO20-23 (*)3.3Vに変換して繋いで下さい
Pin31-33 (PC0-2)   <-- GPIO24-26
   Pin35 (RESET#)  --> GPIO27 (*)3.3Vに変換して繋いで下さい
   Pin36 (GND)     --- GND

加えて、FDDコネクタの9番ピンと10番ピンを繋いでください。これを繋がないと、FDDとして認識されません。
FDD Port         FDD Port
Pin9 (EXTON) --- Pin10 (GND)
```
**注意**：PC側の電圧は5V、Raspberry Pi側の電圧は3.3Vなので、少なくともRaspberry Pi側に入力される上記(\*)の信号は、レベルシフタ、あるいは抵抗による分圧で、5Vから3.3Vへ変換して下さい。
抵抗で分圧する場合は、下記のように接続します（これを12セット）。
```
PC側の信号 -+
           |
          10kΩ
           |
           +-- Raspberry PiのGPIOへ
           |
          20kΩ
           |
          GND
```

## コンパイルのしかた
```
$ git clone https://github.com/MinatsuT/RPi_PC-80S31.git
$ cd RPi_PC-80S31
$ make
```

## 実行のしかた
d88形式のシステムディスクイメージファイルを用意し、下記のコマンドで起動します。
ディスクイメージファイルは、２つまで指定できます。
```
$ sudo ./pc80s31 system.d88
```

また、空(0バイト)のイメージファイルを指定すると、未フォーマットのディスクとして挿入されます。
この場合、接続したPCからフォーマットして下さい。
```
$ touch blank.d88
$ sudo ./pc80s31 system.d88 blank.d88
```

## 免責事項
当該情報に基づいて被ったいかなる損害について、一切責任を負うものではございませんのであらかじめご了承ください。
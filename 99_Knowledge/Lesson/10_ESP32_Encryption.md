# ESP32 Flash Encryption

## 1. Introduction 

In IoT products, firmware security is extremely important.  
Many IoT devices connect to cloud services and store sensitive data such as:

- Device certificates
- Private keys
- WiFi credentials
- Application firmware

If attackers can read the SPI Flash directly, they may:

- Clone the product
- Extract firmware
- Steal certificates
- Modify the firmware

To protect firmware and sensitive data, the ESP32 provides security features through the **ESP-IDF** framework.

The ESP32 provides two main firmware security mechanisms:

- Secure Boot
- Flash Encryption

---

## 2. Secure Boot 

Secure Boot ensures the **authenticity and integrity** of the firmware stored in external SPI Flash.

An attacker could potentially modify the firmware stored in flash memory and run malicious code on the device.

Secure Boot prevents this by creating a **chain of trust**:

```
ROM Bootloader
    ↓
Second-stage Bootloader
    ↓
Application Firmware
```


The firmware must be **digitally signed** using a **private key**.

If the firmware signature is not valid, the ESP32 **will refuse to boot the firmware**.

Therefore:

- Only trusted firmware can run
- Unauthorized firmware cannot be executed

---

## 3. Flash Encryption 

Flash Encryption protects the **confidentiality of data stored in SPI Flash**.

When Flash Encryption is enabled:

- All data stored in SPI Flash is encrypted using **AES-256**
- Physical access to SPI Flash is no longer enough to recover firmware

The following flash regions are encrypted:

- Bootloader
- Partition Table
- Application Firmware
- Any partition marked with the `encrypted` flag

Example encrypted partitions in `partitions.csv`:

| Name | Type | SubType | Offset | Size | Flags |
|------|------|---------|--------|------|-------|
| otadata | data | ota | - | 0x2000 | encrypted |
| phy_init | data | phy | - | 0x1000 | encrypted |
| clientcrt | data | 0xff | 0x1d000 | 0x1000 | encrypted |
| clientkey | data | 0xff | 0x1e000 | 0x1000 | encrypted |
| cacrt | data | 0xff | 0x1f000 | 0x1000 | encrypted |
| deviceinfo | data | 0xff | 0x20000 | 0x4000 | encrypted |


Sensitive files such as:

- TLS certificates
- Private keys
- Device information

can be safely stored in encrypted flash.

---

## 4. Enabling Flash Encryption

Flash encryption is enabled through **ESP-IDF menuconfig**.

### Step 1: Run menuconfig

```bash
idf.py menuconfig
```

### Step 2: Navigate to Security Features

Go to **Security Features** section.

### Step 3: Enable flash encryption

Enable the option: **Enable flash encryption on boot**

<img src="..\Image\Enable_Flash_Encyption.png" >

### Step 4: Set offset of partition table

Set the offset of the partition table to **0x9000**.

<img src="..\Image\Off_of_Partition_table.png" >

---

## 5. Flash Encryption Implementation

### 5.1 Generating Flash Encryption Key

Flash encryption uses a **256-bit AES key**.

Generate the key using the ESP-IDF security tool:

```bash
espsecure.py generate_flash_encryption_key my_flash_encryption_key.bin
```

**Output:**

```
Writing 256 random bits to key file my_flash_encryption_key.bin
```

This file contains the **AES-256 encryption key**.

---

### 5.2 Burning the Encryption Key to eFuse

The encryption key must be stored inside **ESP32 eFuse**.

#### Command:

```bash
espefuse.py --port COM8 burn_key flash_encryption my_flash_encryption_key.bin
```

#### Output example:

```
Burn keys to blocks:
BLOCK1 -> [d3 ae 77 d5 ca 67 20 b9 ...]

Disabling read to key block
Disabling write to key block
```

#### Important Notes:

- The key is written into **eFuse BLOCK1**
- The key becomes **read protected**
- The key becomes **write protected**
- **This operation is IRREVERSIBLE**

#### Confirmation prompt:

```
This is an irreversible operation!
Type 'BURN' to continue
```

#### After confirmation:

```
BURN BLOCK1 - OK
Successful
```

Now the ESP32 contains the **hardware encryption key**.

---

### 5.3 Enabling Flash Encryption eFuses

#### Enable the flash encryption counters:

```bash
espefuse.py -p COM8 burn_efuse FLASH_CRYPT_CNT
```

#### Configure encryption settings:

```bash
espefuse.py -p COM8 burn_efuse FLASH_CRYPT_CONFIG 0xf
```

This enables the hardware encryption engine.

---

### 5.4 Building the Project

#### Build command:

```bash
idf.py build
```

#### Firmware images generated:

```
bootloader.bin
partition-table.bin
flash_encryption.bin
```

---

### 5.5 Flashing Encrypted Firmware

#### Flash with encryption enabled:

```bash
idf.py encrypted-flash monitor
```

#### This internally runs:

```bash
esptool.py write_flash --encrypt
```

#### Flash memory layout:

```
0x1000   bootloader.bin
0x9000   partition-table.bin
0x20000  flash_encryption.bin
```

---

### 5.6 Boot Log After Encryption

#### Example boot log:

```
flash_encrypt: Flash encryption mode is DEVELOPMENT
```

#### ESP32 system information:

```
This is ESP32 chip with 2 CPU cores
FLASH_CRYPT_CNT eFuse value is 1
Flash encryption feature is enabled
```

---

### 5.7 Flash Encryption Verification 

### Writing test data:

```
Writing data with esp_partition_write:
00 01 02 03 04 05 06 07
08 09 0a 0b 0c 0d 0e 0f
```

### Reading with partition API:

```
Reading with esp_partition_read:
00 01 02 03 04 05 06 07
08 09 0a 0b 0c 0d 0e 0f
```

This shows **correct decrypted data**.

### Reading raw flash:

```bash
spi_flash_read
```

Returns **encrypted data**:

```
46 fb f1 1f 1e 4d 63 b4
a1 2d d4 83 dd f9 3b 51
```


This demonstrates that **data stored in SPI flash is encrypted**.

---

## 6. Security Benefits 

Flash Encryption protects against:

- Firmware extraction
- Device cloning
- Certificate theft
- Reverse engineering

Even if attackers dump the SPI Flash, they will only obtain **encrypted data**.

---

## 7. Development vs Production Mode

Two modes are available:

### Development Mode 

- Allows reflashing
- Allows debugging
- Easier for development

**Example log:**

```
Flash encryption mode is DEVELOPMENT
```


### Release Mode 

- Fully secure
- No plaintext flashing
- Strong production security

**Production devices should use Release Mode.**

---

## 8. Conclusion

Flash Encryption is an essential security feature for ESP32 devices.

It protects:

- firmware
- credentials
- certificates
- private keys

By combining:

- Flash Encryption
- Secure Boot

developers can build **secure IoT products resistant to firmware attacks**.
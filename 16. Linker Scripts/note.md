Dưới đây là toàn bộ nội dung chương **Linker Scripts (Kịch bản liên kết)** được thiết kế và định dạng chuẩn dưới dạng file Markdown (`.md`). Bạn có thể lưu lại thành file `linker_scripts.md` để phục vụ việc học tập và tra cứu.


# Chương: Kịch bản Liên kết (Linker Scripts) trong Lập trình Nhúng

# 1. Linker Script là gì? (What they are)
Linker Script (thường có phần mở rộng `.ld` hoặc `.icf`) là một file cấu hình đặc biệt chỉ thị cho **Trình liên kết (Linker)** cách gộp các file đối tượng (`.o` - kết quả sau khi biên dịch các file `.c`) thành một file nhị phân duy nhất (`.elf`, `.bin`, `.hex`) để nạp xuống vi điều khiển.

Nói một cách đơn giản, nếu Trình biên dịch (Compiler) chịu trách nhiệm dịch code của bạn thành các khối gạch đá, thì Linker Script chính là **bản thiết kế kiến trúc** quyết định vị trí đặt từng khối gạch đó vào đúng phân vùng vật lý tương ứng trên chip (như Flash, RAM).



---

## 2. Các thành phần cốt lõi được định nghĩa trong Linker Script

Một file Linker Script tiêu chuẩn thường có 3 phần cơ bản sau:

### Lệnh `MEMORY` (Phân vùng bộ nhớ vật lý)
Định nghĩa chính xác "bản đồ" phần cứng của con chip, bao gồm tên vùng, thuộc tính truy cập (r: đọc, w: ghi, x: thực thi), địa chỉ bắt đầu (`ORIGIN`) và dung lượng tối đa (`LENGTH`).
```ld
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K /* Vùng nhớ Chỉ đọc & Thực thi */
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 128K /* Vùng nhớ Đọc/Ghi dữ liệu */
}

```

### Lệnh `SECTIONS` (Sắp xếp phân đoạn dữ liệu)

Chỉ định các phân đoạn dữ liệu trong code (Output Sections) sẽ được đặt vào vùng nhớ vật lý nào:

* **`.text` (Mã nguồn):** Chứa các lệnh thực thi của CPU và các hằng số `const`, được xếp vào **FLASH**.
* **`.data` (Biến toàn cục có khởi tạo giá trị):** Ví dụ `int x = 10;`. Biến này có giá trị ban đầu nên giá trị ban đầu phải lưu ở **FLASH**, nhưng khi chạy CPU cần thay đổi giá trị nên nó phải chiếm chỗ ở **RAM**. Linker Script sẽ cấu hình kỹ thuật "địa chỉ kép" này (VMA và LMA).
* **`.bss` (Biến toàn cục không khởi tạo/khởi tạo bằng 0):** Ví dụ `int y;`. Vùng này không tốn dung lượng lưu trữ trên Flash, trình biên dịch chỉ cần cấp phát địa chỉ cho nó nằm trên **RAM** và file `startup.s` sẽ tự động xóa sạch vùng này về 0 khi bật nguồn.

### Các Ký hiệu Hệ thống (Symbols)

Tạo ra các biến neo địa chỉ để đoạn code C (đặc biệt là file Startup) có thể gọi và sử dụng, ví dụ: vị trí bắt đầu/kết thúc của Heap (`_end_of_heap`), đỉnh của bộ nhớ Stack (`_estack`).

---

## 3. Tại sao nó lại quan trọng? (Why they matter)

Trong lập trình phần mềm máy tính, hệ điều hành tự động lo liệu việc nạp chương trình vào RAM. Nhưng trong hệ thống nhúng, bạn phải tự tay sắp đặt mọi thứ:

* **Khởi chạy hệ thống đúng quy cách:** Khi vừa cấp điện, vi điều khiển luôn nhảy tới một địa chỉ cố định (Vector Table - Mảng các con trỏ ngắt) được cấu hình nằm ở đầu vùng nhớ Flash. Nếu Linker Script đặt chệch Vector Table này sang vị trí khác, chip sẽ biến thành "cục gạch" (brick) ngay khi bật nguồn.
* **Định hình Stack và Heap:** Linker Script quy định chính xác giới hạn của bộ nhớ Stack và Heap. Nếu cấu hình sai layout, Stack có thể phát triển đè nạp lên các biến `.data`, gây ra các hành vi sai lệch không thể dự đoán hoặc lỗi sập nguồn phần cứng (`HardFault`).

---

## 4. Ví dụ Minh họa một Linker Script Đơn giản (GCC)

```ld
/* Định nghĩa điểm vào của chương trình (Hàm khởi chạy đầu tiên) */
ENTRY(Reset_Handler)

/* Khai báo đỉnh Stack nằm ở cuối phân vùng RAM */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* Cấu hình bản đồ bộ nhớ vật lý */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 256K
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 64K
}

/* Sắp xếp các đoạn dữ liệu vào bộ nhớ */
SECTIONS
{
    /* 1. Đoạn mã nguồn và Vector Table được đặt vào FLASH */
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector)) /* Giữ lại bảng Vector Table không cho tối ưu xóa bỏ */
        *(.text)             /* Gom tất cả các đoạn mã nguồn .text */
        *(.rodata)           /* Gom dữ liệu chỉ đọc (hằng số const) */
        . = ALIGN(4);
    } > FLASH

    /* 2. Biến toàn cục có giá trị khởi tạo */
    /* LMA (địa chỉ lưu trữ) ở FLASH, VMA (địa chỉ thực thi) ở RAM */
    .data : 
    {
        _sdata = .;          /* Ký hiệu: Bắt đầu vùng .data trên RAM */
        *(.data)
        . = ALIGN(4);
        _edata = .;          /* Ký hiệu: Kết thúc vùng .data trên RAM */
    } > RAM AT > FLASH

    /* 3. Biến toàn cục khởi tạo bằng 0 nằm trọn trên RAM */
    .bss :
    {
        _sbss = .;           /* Ký hiệu: Bắt đầu vùng .bss */
        *(.bss)
        . = ALIGN(4);
        _ebss = .;           /* Ký hiệu: Kết thúc vùng .bss */
    } > RAM
}

```

---

## 5. Khi nào bạn cần can thiệp chỉnh sửa Linker Script? (Usage)

Mặc dù các môi trường IDE (như STM32CubeIDE, Keil MDK) luôn tự động sinh file Linker Script mặc định cho bạn, bạn sẽ bắt buộc phải mở file này ra chỉnh sửa thủ công trong các trường hợp nâng cao sau:

1. **Thiết kế hệ thống Bootloader:** Bạn cần chia đôi bộ nhớ Flash vật lý thành 2 phân vùng độc lập: Một phân vùng nhỏ phía đầu chứa code Bootloader, phần còn lại chứa code Ứng dụng chính (Application).
2. **Tạo phân vùng hiệu chuẩn (Calibration/Configuration Data):** Bạn muốn dành riêng một ô nhớ Flash cố định (ví dụ Sector cuối cùng của Flash) để lưu các thông số cấu hình, mã ID của thiết bị mà không sợ bị trình biên dịch ghi đè mã nguồn lên đó.
3. **Mở rộng bộ nhớ ngoài (External RAM/Flash):** Khi thiết kế bo mạch mở rộng thêm IC RAM ngoài (như SDRAM) hoặc Chip Flash (qua giao tiếp QSPI), bạn phải khai báo thêm các phân vùng bộ nhớ này vào lệnh `MEMORY` thì mới có thể sử dụng được.

---

## 6. Câu hỏi Thực tế (Practical Questions)

### Q1: Địa chỉ LMA và VMA trong phân đoạn `.data` khác nhau như thế nào? Tại sao file Startup phải cần đến các ký hiệu như `_sdata` và `_edata`?

* **Trả lời:** * **LMA (Load Memory Address):** Là địa chỉ nơi dữ liệu thực sự **nằm vật lý** khi chip bị mất điện. Đối với biến có giá trị khởi tạo (như `int x = 10;`), con số `10` này bắt buộc phải được lưu cố định trong **FLASH** (LMA).
* **VMA (Virtual Memory Address):** Là địa chỉ nơi dữ liệu **vận hành khi chương trình chạy**. Vì CPU cần thay đổi giá trị của `x`, con số `10` đó phải được copy sang **RAM** (VMA) để tính toán.
* *Vai trò của file Startup:* Khi vi điều khiển vừa bật nguồn, vùng RAM hoàn toàn trống rác. File `startup.s` sẽ chạy một đoạn code vòng lặp `for` sử dụng các ký hiệu do Linker Script cung cấp là `_sdata` (địa chỉ RAM bắt đầu) và `_edata` (địa chỉ RAM kết thúc) để tiến hành **bốc toàn bộ các giá trị khởi tạo ban đầu đang nằm ở FLASH và copy nạp vào RAM**. Nếu không có bước cấu hình LMA/VMA và vòng lặp copy này, tất cả các biến toàn cục có khởi tạo của bạn sẽ mang giá trị rác khi chạy.



### Q2: Điều gì xảy ra nếu tổng dung lượng khai báo của các biến `.text`, `.data`, `.bss` vượt quá kích thước vật lý ghi trong Linker Script?

* **Trả lời:** Hệ thống sẽ kích hoạt một lỗi biên dịch ngay lập tức ở giai đoạn liên kết, thường có thông báo dạng: `region 'RAM' overflowed by X bytes` hoặc `region 'FLASH' overflowed`. Lỗi này cực kỳ an toàn vì trình liên kết (Linker) đã chủ động phát hiện ra rằng tổng không gian mà phần mềm yêu cầu đã vượt quá ranh giới vật lý (`LENGTH`) của phần cứng con chip. Nó sẽ chặn đứng lại và không xuất ra file nhị phân (`.bin`), giúp bạn phòng tránh được hoàn toàn nguy cơ nạp một chương trình lỗi cấu trúc xuống phần cứng khiến chip bị treo.



Khi bạn viết code C và bấm biên dịch, trình biên dịch sẽ băm toàn bộ mã nguồn của bạn (biến, hàm, hằng số) thành các phân vùng chuẩn sau đây:

---

## 1. Bản đồ phân chia dữ liệu lên các phân vùng bộ nhớ

Dưới đây là bảng tra cứu nhanh giúp bạn hình dung loại dữ liệu nào sẽ đi vào phân vùng nào:

| Tên phân vùng | Vị trí vật lý | Loại dữ liệu chứa bên trong | Trạng thái khi mất điện |
| --- | --- | --- | --- |
| **`.text`** | **Flash / ROM** | Mã máy (chứa các câu lệnh), cấu trúc hàm logic. | **Không mất** |
| **`.rodata`** | **Flash / ROM** | Dữ liệu chỉ đọc (`const`), chuỗi ký tự cố định. | **Không mất** |
| **`.data`** | **RAM** *(Gốc ở Flash)* | Biến toàn cục/static có khởi tạo giá trị **khác 0**. | **Mất** (Phải nạp lại) |
| **`.bss`** | **RAM** | Biến toàn cục/static không khởi tạo hoặc **bằng 0**. | **Mất** (Tự xóa về 0) |
| **`Stack`** | **RAM** | Biến cục bộ trong hàm, địa chỉ ngắt, tham số hàm. | **Mất** (Tự tăng/giảm) |
| **`Heap`** | **RAM** | Bộ nhớ cấp phát động dùng `malloc()` / `calloc()`. | **Mất** (Do user quản lý) |

---

## 2. Chi tiết từng phân vùng và Data tương ứng (Ví dụ bằng Code C)

Hãy nhìn vào đoạn code C thực tế dưới đây để thấy cách các phân vùng "hút" dữ liệu tương ứng của chúng:

```c
#include <stdint.h>
#include <stdlib.h>

// 1. Phân vùng .rodata (Read-Only Data)
const uint32_t MAX_SPEED = 240;           // Nằm ở .rodata (Flash)
const char* welcome_msg = "Hello World";   // Bản thân chuỗi "Hello World" nằm ở .rodata (Flash)

// 2. Phân vùng .data (Initialized Data)
int32_t system_status = 1;                 // Biến toàn cục có giá trị khác 0 -> .data (RAM)
static uint8_t sensor_mode = 0xAA;         // Biến static có giá trị khác 0 -> .data (RAM)

// 3. Phân vùng .bss (Block Started by Symbol)
int32_t error_count;                       // Toàn cục, không khởi tạo (mặc định = 0) -> .bss (RAM)
static uint8_t rx_buffer[64] = {0};        // Biến static khởi tạo bằng 0 -> .bss (RAM)

// Phân vùng .text (Mã máy của các hàm)
void control_motor(int speed) {            // Toàn bộ logic của hàm này nằm ở .text (Flash)
    
    // 4. Phân vùng STACK (Dữ liệu tạm thời)
    int32_t temp_calculation = speed * 2;  // Biến cục bộ -> Nằm trên STACK (RAM)
    uint8_t local_array[10];               // Mảng cục bộ -> Nằm trên STACK (RAM)
    
    // 5. Phân vùng HEAP (Cấp phát động)
    uint8_t *dynamic_buf = malloc(20);     // Vùng nhớ 20 bytes được trích từ HEAP (RAM)
    
    free(dynamic_buf);
}

```

---

## 3. Bản chất vận hành của ".data" và ".bss" tại lúc khởi động

Đây là phần thú vị nhất và cũng là lý do tại sao Linker Script lại phức tạp:

### Tại sao `.data` cần "địa chỉ kép" (LMA và VMA)?

Như biến `system_status = 1` ở ví dụ trên:

* Khi bạn **tắt nguồn** vi điều khiển, con số `1` này bắt buộc phải lưu ở **Flash** (Địa chỉ lưu trữ - LMA). Nếu không, khi bật lại chip làm sao nó biết trạng thái ban đầu là `1`?
* Khi chip **hoạt động**, CPU cần cộng, trừ, thay đổi biến này. Flash thì không thể ghi xóa linh hoạt được, nên con số `1` này phải được ánh xạ sang **RAM** (Địa chỉ vận hành - VMA).
* **Data đi vào:** Do đó, file Startup (`startup.s`) khi vừa bật nguồn sẽ chạy một vòng lặp, bốc toàn bộ giá trị khởi tạo ban đầu từ Flash để **copy nạp đầy vào vùng `.data` trên RAM**.

### Tại sao `.bss` không tốn dung lượng Flash?

Biến `rx_buffer[64] = {0};` chiếm tới 64 bytes RAM.

* Nếu trình biên dịch lưu 64 con số 0 này vào Flash thì quá lãng phí bộ nhớ Flash.
* Vì vậy, Linker Script chỉ lưu **địa chỉ bắt đầu và độ dài** của vùng `.bss`.
* Khi bật nguồn, file Startup chỉ cần chạy một vòng lặp `memset` để **xóa sạch toàn bộ phân vùng `.bss` trên RAM về 0**. Đó là lý do tại sao biến toàn cục không khởi tạo trong C luôn tự động bằng 0.

---

## 4. Phân vùng mở rộng tùy biến (Custom Sections)

Ngoài các phân vùng tiêu chuẩn trên, bạn hoàn toàn có thể tự tạo ra các phân vùng riêng bằng thuộc tính `__attribute__((section("tên_phân_vùng")))` trong code C để ép dữ liệu vào một vị trí cố định được quy định trong Linker Script.

* **Phân vùng `.isr_vector`:** Phân vùng này chứa mảng các con trỏ hàm ngắt (Vector Table). Nó bắt buộc phải được đặt ở **địa chỉ đầu tiên của Flash (thường là `0x08000000`)**, vì phần cứng CPU khi reset luôn nhìn vào đây đầu tiên.
* **Phân vùng `.eeprom_data` hoặc `.calib_data`:** Bạn tạo ra để chứa các cấu trúc struct cấu hình thiết bị, ép Linker Script đặt nó ở Sector cuối cùng của Flash để làm nhiệm vụ lưu trữ giống như EEPROM, không bị ghi đè khi nạp code mới.

Phần phân chia chi tiết này đã giúp bạn làm rõ được "đường đi nước bước" của từng dòng code C khi chuyển hóa thành các ô nhớ chưa?
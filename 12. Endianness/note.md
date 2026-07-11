# Chương: Định dạng Byte (Endianness) trong Lập trình Nhúng

## 1. Endianness là gì? (What it is)
Endianness là thuật ngữ dùng để chỉ **thứ tự sắp xếp các byte** của một kiểu dữ liệu gồm nhiều byte (như `uint16_t`, `uint32_t`, `float`) khi chúng được lưu trữ vào bộ nhớ RAM hoặc khi truyền qua đường dây tín hiệu.

Khi ta có một giá trị nhiều byte, ví dụ số nguyên 4-byte dạng Hex: `0x12345678`. Số này được chia thành 2 đầu:
* **MSB (Most Significant Byte - Byte có trọng số cao nhất):** là byte `0x12`.
* **LSB (Least Significant Byte - Byte có trọng số thấp nhất):** là byte `0x78`.

Có hai cách sắp xếp các byte này vào bộ nhớ tương ứng với hai loại Endianness:

| Loại Endianness | Quy tắc sắp xếp | Thứ tự byte trong RAM (Địa chỉ từ thấp đến cao) | Kiến trúc phổ biến |
| :--- | :--- | :--- | :--- |
| **Little-endian** | Byte có trọng số thấp nhất (**LSB**) được ưu tiên xếp vào **địa chỉ thấp nhất**. | `0x78`, `0x56`, `0x34`, `0x12` | x86 (Intel/AMD), đa số lõi ARM Cortex-M. |
| **Big-endian** | Byte có trọng số cao nhất (**MSB**) được ưu tiên xếp vào **địa chỉ thấp nhất**. | `0x12`, `0x34`, `0x56`, `0x78` | Giao thức mạng Internet (TCP/IP), một số chip DSP. |



---

## 2. Tại sao nó lại quan trọng trong Hệ thống Nhúng? (Implications)

Trong nội bộ một vi điều khiển (MCU) sử dụng cấu trúc Little-endian, việc tính toán hay đọc/ghi biến diễn ra hoàn toàn tự động và chính xác. Tuy nhiên, Endianness trở thành một "bẫy lỗi" nguy hiểm khi bạn thực hiện **truyền nhận dữ liệu thô (raw bytes) giữa các thiết bị khác nhau**:

* **Giao tiếp Đa nền tảng (Host-MCU Communication):** Khi bạn gửi một biến `uint32_t` từ một mạch vi điều khiển chạy lõi Big-endian sang một máy tính chạy chip Intel (Little-endian) qua cổng UART/SPI. Nếu không chuyển đổi, con số nhận được ở máy tính sẽ bị đảo ngược hoàn toàn.
* **Quy chuẩn mạng (Network Byte Order):** Tất cả các giao thức mạng chuẩn Internet (như TCP/IP, UDP) đều quy định dữ liệu truyền trên đường truyền bắt buộc phải tuân theo định dạng **Big-endian**. Thiết bị nhúng dùng Little-endian khi muốn kết nối Internet bắt buộc phải đổi định dạng byte trước khi gửi.
* **Đọc file cấu hình nhị phân (Binary Files):** Các file ảnh (BMP, PNG) hay file âm thanh (WAV) lưu trên thẻ nhớ SD thường có quy định nghiêm ngặt về Endianness cho các thẻ tiêu đề (headers). Đọc sai quy chuẩn sẽ làm hỏng dữ liệu.

---

## 3. Ví dụ Minh họa bằng Code C

### Kỹ thuật đóng gói tường minh (Explicit Packing) khi truyền dữ liệu qua UART

Giả sử bạn có một biến `uint16_t value = 0x1234;` và bạn cần gửi nó qua giao tiếp UART sao cho phía nhận (quy định nhận dạng Big-endian) hiểu đúng.

```c
#include <stdint.h>

// Hàm giả định gửi 1 byte qua phần cứng UART
void send_byte(uint8_t byte);

void transmit_data(void) {
    uint16_t value = 0x1234;

    // Nếu phía nhận yêu cầu định dạng Big-endian (MSB đi trước, LSB đi sau):
    send_byte((value >> 8) & 0xFF);  // Trích xuất và gửi byte cao trước (0x12)
    send_byte(value & 0xFF);         // Trích xuất và gửi byte thấp sau (0x34)
}
# Chương: Căn lề Bộ nhớ (Memory Alignment) trong Lập trình Nhúng

## 1. Memory Alignment là gì? (What it is)
Căn lề bộ nhớ là quy tắc sắp xếp địa chỉ dữ liệu của CPU. Hãy hình dung RAM là một mảng lớn gồm các ô nhớ 1-byte nối tiếp nhau với địa chỉ $0, 1, 2, 3, \dots$

Hầu hết các CPU (đặc biệt là lõi ARM Cortex-M trong vi điều khiển) không đọc dữ liệu theo từng byte đơn lẻ mà đọc theo từng từ mã (**Word** - thường là 4 bytes một lúc). Do đó, CPU yêu cầu:
* Một kiểu dữ liệu 2-byte (như `uint16_t`) phải bắt đầu tại một **địa chỉ chẵn** (chia hết cho 2).
* Một kiểu dữ liệu 4-byte (như `uint32_t`) phải bắt đầu tại một **địa chỉ chia hết cho 4**.

Khi dữ liệu tuân thủ đúng các quy tắc này, ta gọi đó là bộ nhớ đã được **Căn lề (Aligned)**. Ngược lại, nếu một biến `uint32_t` nằm ở địa chỉ lẻ (ví dụ `0x20000001`), nó được gọi là **Lệch hàng (Misaligned)**.



---

## 2. Tại sao nó lại quan trọng trong Hệ thống Nhúng? (Why it matters)

* **Tối ưu hiệu năng (Performance):** Nếu dữ liệu được căn lề đúng vị trí, CPU chỉ tốn đúng **1 chu kỳ bus (1 bus cycle)** để lấy được giá trị. Nếu dữ liệu bị lệch hàng và nằm vắt ngang ranh giới của 2 ô Word, CPU sẽ phải thực hiện **2 lần đọc** bộ nhớ, sau đó dùng các lệnh dịch bit để dịch chuyển và ghép dữ liệu lại $\rightarrow$ Làm giảm đáng kể tốc độ thực thi.
* **Tránh sập nguồn hệ thống (Correctness):** Một số dòng vi điều khiển (hoặc một số kiến trúc CPU cũ) hoàn toàn **không hỗ trợ** đọc/ghi lệch hàng. Việc cố tình truy cập vào một địa chỉ `uint32_t` không chia hết cho 4 sẽ kích hoạt ngay lập tức một lỗi ngắt phần cứng nghiêm trọng (**UsageFault** hoặc **HardFault**).
* **Yêu cầu bắt buộc của Ngoại vi (Hardware requirements):** Các bộ điều khiển chuyển đổi dữ liệu tự động như **DMA (Direct Memory Access)**, Ethernet, USB đòi hỏi các bộ đệm (buffers) truyền nhận phải được căn lề chính xác (thường là mốc chia hết cho 4 hoặc 8 bytes) để mạch phần cứng có thể nạp dữ liệu trực tiếp với tốc độ cao.
* **Giải thích cấu trúc Struct (Predictable layout):** Hiểu về alignment giúp bạn lý giải được tại sao kích thước của một `struct` (`sizeof`) trong C thường lớn hơn tổng kích thước của các thành phần bên trong nó cộng lại. Đó là do trình biên dịch tự động chèn thêm các **Byte đệm (Padding bytes)** để đảm bảo các phần tử phía sau được căn lề đúng vị trí.

---

## 3. Các rủi ro kinh điển và Cách phòng ngừa

### 1. Ép kiểu con trỏ sai quy cách (Pointer Casting Fault)
Lỗi này xảy ra khi bạn nhận được một mảng byte thô (`uint8_t *`) từ môi trường ngoài, sau đó ép kiểu nó thành một con trỏ có kích thước lớn hơn một cách bừa bãi:
```c
uint8_t rx_buffer[10];
// Giả sử dữ liệu thực sự bắt đầu từ byte số 1 (địa chỉ lẻ)
uint32_t *data_ptr = (uint32_t *)&rx_buffer[1]; 
uint32_t value = *data_ptr; // ❌ NGUY HIỂM: Có thể gây HardFault lập tức trên MCU!
```











```c


#include <stdint.h>
#include <stdio.h>

// Struct A: Sắp xếp lộn xộn các kiểu dữ liệu
struct A {
    uint8_t  a;  // 1 byte
    // Trình biên dịch sẽ chèn 3 bytes PADDING ở đây để 'b' nằm ở địa chỉ chia hết cho 4
    uint32_t b;  // 4 bytes
    uint8_t  c;  // 1 byte
    // Trình biên dịch chèn tiếp 3 bytes PADDING ở cuối để đảm bảo mảng struct (nếu có) luôn aligned
}; // Tổng kích thước thực tế: 12 bytes! (Dù dữ liệu thực chỉ có 6 bytes)

// Struct B: Sắp xếp tối ưu, từ lớn đến bé
struct B {
    uint32_t b;  // 4 bytes - Đặt trường lớn nhất lên đầu
    uint8_t  a;  // 1 byte
    uint8_t  c;  // 1 byte
    // Chỉ chèn 2 bytes PADDING ở cuối để làm tròn thành mốc 8 bytes
}; // Tổng kích thước thực tế: 8 bytes! (Tiết kiệm được 4 bytes RAM so với Struct A)

int main(void) {
    printf("sizeof(struct A) = %zu\n", sizeof(struct A));  // Kết quả: 12
    printf("sizeof(struct B) = %zu\n", sizeof(struct B));  // Kết quả: 8

    /* Ví dụ về truy cập Lệch hàng (Misaligned Access) - CẤM DÙNG TRÊN MCU */
    uint8_t buf[8] = {0};
    // buf[1] nằm ở địa chỉ lẻ, chắc chắn không chia hết cho 4
    uint32_t *p32 = (uint32_t *)&buf[1];  
    
    // Câu lệnh này có thể gây crash (HardFault) hoặc chạy rất chậm tùy loại CPU
    *p32 = 0x11223344; 

    return 0;
}



```
Mẹo lập trình thực tế (Usage Tips)

    Quy tắc sắp xếp Struct: Hãy luôn luôn khai báo các trường dữ liệu có kích thước lớn hơn lên phía trước (ví dụ xếp uint32_t đầu tiên, rồi đến uint16_t, và cuối cùng mới là uint8_t). Quy tắc này tự động triệt tiêu phần lớn các byte padding dư thừa, giúp tối ưu RAM tối đa.

    Ép căn lề cho Bộ đệm Ngoại vi: Khi tạo mảng làm bộ đệm cho DMA, hãy sử dụng các thuộc tính (attributes) đặc trưng của trình biên dịch để ép mảng đó nằm ở địa chỉ chuẩn:

        Với GCC / Clang: uint8_t dma_buf[256] __attribute__((aligned(4)));

        Với Keil C (ARMCC): __align(4) uint8_t dma_buf[256];

    Đóng gói Struct khi truyền dữ liệu (Packed Struct): Nếu bắt buộc phải truyền struct qua môi trường truyền thông hoặc lưu vào Flash/EEPROM, hãy sử dụng chỉ thị __attribute__((packed)) hoặc #pragma pack(1) để ép trình biên dịch xóa bỏ hoàn toàn các byte padding.

        Lưu ý quan trọng: Khi một struct đã bị packed, việc truy cập vào các trường bên trong nó có thể trở thành truy cập lệch hàng. Vì vậy, tốt nhất hãy viết các hàm copy thủ công từng byte (serialize/deserialize) thay vì ép kiểu con trỏ trực tiếp vào struct.
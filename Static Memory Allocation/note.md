# Chương: Cấp phát Bộ nhớ Tĩnh (Static Memory Allocation) trong Lập trình Nhúng

## 1. Cấp phát Bộ nhớ Tĩnh là gì? (What it is)
Cấp phát bộ nhớ tĩnh là cơ chế mà trong đó kích thước (size) và địa chỉ ô nhớ (location) của các biến được **xác định và cố định ngay từ giai đoạn biên dịch và liên kết (Compile/Link time)**. 

Vùng bộ nhớ này tồn tại suốt toàn bộ vòng đời của chương trình, bao gồm:
* Các biến toàn cục (Global variables).
* Các biến cục bộ có từ khóa `static` (Local static variables).
* Các mảng/bộ đệm được khai báo cố định kích thước.



---

## 2. Tại sao Bộ nhớ Tĩnh là "tiêu chuẩn vàng" trong Nhúng? (Why it matters)

Trong lập trình phần mềm máy tính, việc dùng `malloc()` hay `new` (Cấp phát động - Dynamic Allocation) diễn ra rất phổ biến. Nhưng trong hệ thống nhúng – đặc biệt là các hệ thống an toàn cao (Safety-critical) như y tế, ô tô, hàng không – cấp phát động thường bị **cấm hoàn toàn** vì những lý do sau:

* **Tính toàn vẹn và Dự đoán được (Predictable layout):** Vì vị trí và kích thước biến được chốt ngay từ lúc biên dịch, bạn luôn biết chắc chắn hệ thống của mình có đủ RAM để chạy hay không ngay khi vừa build xong code. Tránh được hoàn toàn lỗi sập nguồn giữa chừng do thiếu RAM.
* **Không bị phân mảnh bộ nhớ (No fragmentation):** Cấp phát động (`malloc`/`free`) liên tục sẽ tạo ra các "lỗ hổng" bộ nhớ nằm rải rác trong RAM. Sau một thời gian chạy, hệ thống có thể bị từ chối cấp phát dù tổng dung lượng RAM trống vẫn đủ. Bộ nhớ tĩnh triệt tiêu hoàn toàn rủi ro này.
* **Không tốn tài nguyên quản lý (No runtime allocator):** Việc quản lý Heap (vùng nhớ cấp phát động) đòi hỏi CPU phải chạy một thuật toán tìm kiếm ô nhớ trống mỗi khi gọi `malloc`. Bộ nhớ tĩnh giúp CPU không tốn bất kỳ chu kỳ xung nhịp nào cho việc này, đáp ứng thời gian thực (Hard real-time) một cách tuyệt đối.

---

## 3. Ví dụ minh họa bằng Code C

```c
#include <stdint.h>

// 1. Bộ đệm nhận UART cố định 256 bytes trong RAM từ lúc biên dịch
static uint8_t uart_rx_buffer[256];   

// 2. Biến đếm hệ thống cố định vị trí, chỉ hiển thị nội bộ trong file này
static uint32_t tick_count;

void isr_tick(void) { 
    tick_count++; // Tăng biến đếm an toàn trong hàm ngắt
}

```

Q1: Khi nào tôi nên chọn một bộ đệm tĩnh (Static Buffer) thay vì dùng malloc?

    Trả lời: Bạn nên chọn bộ đệm tĩnh luôn luôn và ngay khi có thể, miễn là bạn xác định được kích thước tối đa (Maximum size) của dữ liệu tại thời điểm viết code.

    Ví dụ: Khi bạn làm việc với một module GPS truyền chuỗi dữ liệu NMEA có độ dài tối đa là 82 ký tự, hãy khai báo thẳng một bộ đệm tĩnh: static uint8_t gps_buffer[90];. Việc này giúp loại bỏ hoàn toàn nguy cơ hàm malloc() trả về con trỏ NULL (gây sập hệ thống/HardFault lúc runtime) nếu bộ nhớ Heap xảy ra sự cố.

Q2: Làm thế nào để tránh lãng phí RAM khi khai báo quá nhiều bộ đệm tĩnh?

Việc dùng bộ nhớ tĩnh có nhược điểm là mảng luôn chiếm chỗ trong RAM ngay cả khi không dùng tới. Để tối ưu hóa, bạn có thể áp dụng các chiến lược sau:

    Tái sử dụng bộ đệm (Buffer Sharing): Nếu dự án có nhiều tác vụ không bao giờ chạy cùng một lúc (ví dụ: Task cấu hình Wifi khi khởi động và Task lưu dữ liệu vào thẻ SD khi chạy), hãy cho chúng dùng chung một bộ đệm tĩnh mang tính chất toàn cục (Global Scratchpad Buffer).

    Chuyển sang Flash (static const): Với các mảng dữ liệu lớn chỉ đọc (như bảng mã ASCII, Font màn hình OLED, cấu hình mặc định), bắt buộc phải thêm const để trình biên dịch đẩy chúng sang bộ nhớ Flash, giữ cho RAM luôn trống trải.

    Cấu hình bằng Macro (#define): Sử dụng các hằng số tiền xử lý để định nghĩa kích thước bộ đệm. Khi chuyển đổi dự án giữa các dòng chip có RAM lớn/nhỏ khác nhau, bạn chỉ cần tinh chỉnh lại các giá trị #define này ở một file cấu hình duy nhất (config.h) để tối ưu dung lượng cho từng phiên bản phần cứng.
# Chương: Cấp phát Bộ nhớ Động (Dynamic Memory Allocation) trong Lập trình Nhúng

## 1. Cấp phát Bộ nhớ Động là gì? (What it is)
Cấp phát bộ nhớ động là cơ chế xin cấp phát và giải phóng các vùng nhớ **ngay trong quá trình chương trình đang chạy (Runtime)**. Vùng bộ nhớ dành riêng cho việc này được gọi là **Heap**.

Khác với bộ nhớ tĩnh (vị trí cố định từ lúc biên dịch), kích thước của bộ nhớ động có thể thay đổi linh hoạt tùy theo nhu cầu thực tế của hệ thống tại từng thời điểm thông qua các hàm thư viện chuẩn `<stdlib.h>`.



---

## 2. Các hàm Quản lý Heap tiêu chuẩn (Standard Heap Functions)

* `void *malloc(size_t size);`
  * **Chức năng:** Cấp phát một khối bộ nhớ có kích thước `size` (tính bằng byte).
  * **Đặc điểm:** Dữ liệu bên trong khối nhớ này **chưa được khởi tạo** (chứa rác). Trả về `NULL` nếu thất bại.
* `void *calloc(size_t n, size_t size);`
  * **Chức năng:** Cấp phát vùng nhớ cho `n` phần tử, mỗi phần tử có kích thước `size` byte.
  * **Đặc điểm:** Tự động **xóa toàn bộ vùng nhớ về 0** (Zero-initialized). Trả về `NULL` nếu thất bại.
* `void *realloc(void *ptr, size_t new_size);`
  * **Chức năng:** Thay đổi kích thước của khối bộ nhớ `ptr` đã được cấp phát trước đó sang `new_size` mới.
  * **Đặc điểm:** Có thể di chuyển toàn bộ khối dữ liệu sang một vị trí mới trên Heap nếu không đủ không gian liền kề. Nếu thất bại, trả về `NULL` và **giữ nguyên khối bộ nhớ gốc**.
* `void free(void *ptr);`
  * **Chức năng:** Giải phóng khối bộ nhớ đã cấp phát để đưa về Heap.
  * **Đặc điểm:** Truyền con trỏ `NULL` vào hàm này là an toàn và không thực hiện hành động nào.

---

## 3. Thách thức lớn trong Hệ thống Nhúng (Challenges in Embedded)

Mặc dù rất linh hoạt, việc sử dụng Heap trong lập trình nhúng giống như "chơi với dao hai lưỡi" và thường bị hạn chế tối đa vì 4 lý do:

* **RAM giới hạn (Limited RAM):** Vi điều khiển có rất ít RAM, việc lạm dụng Heap rất dễ dẫn đến cạn kiệt bộ nhớ.
* **Phân mảnh bộ nhớ (Fragmentation):** Việc cấp phát và giải phóng liên tục các khối nhớ có kích thước khác nhau sẽ biến Heap thành một "bãi mìn" vụn vặt. Nhiều mảnh bộ nhớ trống nhỏ nằm rải rác, khiến một lệnh `malloc()` dung lượng lớn tiếp theo bị thất bại dù tổng dung lượng trống của Heap vẫn đủ.
* **Tính phi định thời (Non-determinism):** Thời gian thực thi của hàm `malloc()` phụ thuộc vào việc thuật toán tìm kiếm trên Heap mất bao lâu để tìm thấy một khoảng trống phù hợp. Điều này cực kỳ nguy hiểm cho các hệ thống thời gian thực khắt khe (Hard Real-time) vì tốc độ đáp ứng không đồng đều.
* **Độ tin cậy thấp (Reliability):** Rất khó để phát hiện và gỡ lỗi (debug) các lỗi rò rỉ bộ nhớ hoặc giải phóng hai lần (double-free) trong các hệ thống nhúng chạy liên tục hàng tháng, hàng năm.

> **Quy tắc cốt lõi:** Chỉ dùng bộ nhớ động khi **bắt buộc** (ví dụ: xử lý các gói tin mạng có độ dài biến đổi, giao thức luồng dữ liệu phức tạp). Hãy ưu tiên cấp phát tĩnh hoặc dùng Memory Pool (Bể chứa bộ nhớ).

---

## 4. Hướng dẫn Sử dụng An toàn & Phòng tránh Rủi ro

### Quy tắc viết code an toàn:
1. **Luôn kiểm tra `NULL`** ngay sau khi gọi `malloc`, `calloc` hoặc `realloc`.
2. **Sử dụng con trỏ tạm thời** khi dùng `realloc` để tránh làm mất dấu (losing pointer) vùng nhớ cũ nếu việc mở rộng thất bại.
3. **Quy định rõ quyền sở hữu (Clear ownership):** Xác định rõ hàm nào chịu trách nhiệm cấp phát và hàm nào chịu trách nhiệm `free()`.
4. **Xóa con trỏ sau khi giải phóng:** Luôn gán `ptr = NULL;` ngay sau lệnh `free(ptr);` để tránh lỗi Con trỏ lơ lửng (Dangling pointer / Use-after-free).

### Các rủi ro kinh điển và cách phòng tránh:
* **Memory Leaks (Rò rỉ bộ nhớ):** Quên `free()` làm Heap thu hẹp dần cho đến khi sập nguồn. 
  * *Cách tránh:* Đảm bảo giải phóng bộ nhớ ở mọi nhánh rẽ của code (bao gồm cả các nhánh xử lý lỗi `if/else`).
* **Stack Overflow (Tràn bộ nhớ Stack):** Khác hoàn toàn với tràn Heap. Lỗi này xảy ra khi bạn khai báo các mảng cục bộ quá lớn hoặc đệ quy quá sâu trong hàm, làm phân vùng Stack lấn sang phân vùng khác.
  * *Cách tránh:* Đổi các mảng cục bộ lớn trong hàm thành mảng `static` hoặc cấp phát động trên Heap.

---

## 5. Ví dụ Minh họa bằng Code C

### Ví dụ 1: Sử dụng `malloc` + `free` cơ bản
```c
#include <stdlib.h>
#include <stdio.h>

int process_samples(size_t n) {
    // 1. Cấp phát mảng động dựa trên số mẫu n nhận vào lúc runtime
    int *samples = malloc(n * sizeof(*samples));
    
    // 2. BẮT BUỘC: Kiểm tra xem Heap có đủ bộ nhớ không
    if (samples == NULL) {
        return -1; // Xử lý lỗi thiếu bộ nhớ (Out-of-memory)
    }

    for (size_t i = 0; i < n; ++i) {
        samples[i] = (int)i;
    }

    // 3. Giải phóng bộ nhớ ngay khi hoàn thành công việc
    free(samples);
    samples = NULL; // Tránh bug use-after-free
    
    return 0;
}

```

Q1: Có nên gọi hàm malloc hoặc free bên trong một Hàm ngắt (ISR) không?

    Trả lời: Trong hầu hết tất cả các hệ thống nhúng, câu trả lời tuyệt đối là KHÔNG.

        Các bộ cấp phát bộ nhớ (Allocators) của thư viện chuẩn thường không có tính tái nhập (Non-reentrant). Nếu luồng chính đang chạy malloc nửa chừng và bị ngắt bởi ISR, rồi ISR lại tiếp tục gọi malloc, cấu trúc dữ liệu bên trong Heap sẽ bị phá hủy hoàn toàn (Corruption).

        Hàm malloc/free tiêu tốn thời gian thực thi rất lâu và không cố định, nó có thể khóa các ngắt khác, vi phạm nghiêm trọng nguyên tắc cốt lõi của ISR: "Vào nhanh, ra nhanh".

Q2: Làm cách nào để có được sự linh hoạt như malloc nhưng vẫn giữ cho việc sử dụng bộ nhớ có tính dự đoán trước (Predictable)?

    Trả lời: Giải pháp tối ưu nhất cho hệ thống nhúng là sử dụng Fixed-size Memory Pool (Bể chứa bộ nhớ kích thước cố định).

        Cách hoạt động: Bạn khai báo trước một mảng tĩnh (static array) lớn ở giai đoạn biên dịch. Sau đó, bạn chia mảng lớn này thành nhiều khối nhỏ có kích thước bằng nhau (ví dụ: chia thành 10 khối, mỗi khối 64 bytes).

        Lợi ích: Khi cần dùng, bạn chỉ việc lấy ra một khối trống và khi dùng xong thì trả lại khối đó. Cơ chế này đảm bảo thời gian cấp phát cực kỳ nhanh và cố định (Bounded time), đồng thời triệt tiêu hoàn toàn lỗi phân mảnh bộ nhớ vì mọi khối nhớ đều có cùng một kích cỡ.
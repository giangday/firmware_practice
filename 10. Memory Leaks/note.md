# Chương: Rò rỉ Bộ nhớ (Memory Leaks) trong Lập trình Nhúng

## 1. Memory Leak là gì? (What it is)
Rò rỉ bộ nhớ là hiện tượng chương trình xin cấp phát các khối nhớ trên Heap (bằng `malloc`, `calloc`, hoặc `realloc`) nhưng **không bao giờ giải phóng chúng** (bằng `free`) sau khi đã dùng xong, hoặc làm mất dấu con trỏ duy nhất quản lý vùng nhớ đó.

Theo thời gian, dung lượng Heap trống sẽ **thu hẹp dần** (bị "ăn" mất). Khi hệ thống nhúng vận hành liên tục (chạy hàng tuần, hàng tháng), bộ nhớ Heap sẽ cạn kiệt, dẫn đến các lệnh `malloc` tiếp theo bị từ chối, gây sập hệ thống hoặc đứng máy hoàn toàn.



---

## 2. Các kịch bản gây rò rỉ bộ nhớ phổ biến (How it happens)

* **Bỏ sót `free()` ở nhánh xử lý thành công:** Cấp phát ra để dùng nhưng khi hoàn thành tác vụ thì quên không gọi `free()`.
* **Bỏ sót `free()` ở nhánh xử lý lỗi (Early Return):** Đây là lỗi phổ biến nhất. Bạn cấp phát bộ nhớ ở đầu hàm, sau đó kiểm tra điều kiện phần cứng bên dưới, nếu phát hiện lỗi (`if (error_path)`) liền lập tức `return -1;` mà quên giải phóng khối nhớ vừa tạo.
* **Làm mất dấu con trỏ (Lost Pointer):** Ghi đè địa chỉ mới lên con trỏ đang giữ vùng nhớ Heap cũ mà chưa giải phóng vùng nhớ cũ đó.
* **Mơ hồ về vòng đời biến (Mismatched lifetime):** Một module ngắn hạn (short-lived) cấp phát bộ nhớ rồi truyền sang một module dài hạn (long-lived), nhưng không quy định rõ bên nào có trách nhiệm phải giải phóng, dẫn đến việc cả hai bên đều bỏ mặc.

---

## 3. Quy tắc Phòng ngự: Nên và Không nên (Do's and Don'ts)

### Việc NÊN làm (Good practices):
* **Ưu tiên cấp phát tĩnh:** Nếu biết trước kích thước và số lượng tối đa, hãy dùng mảng tĩnh (`static`) hoặc Memory Pool thay vì dùng Heap.
* **Định nghĩa rõ quyền sở hữu (Clear ownership):** Ngay khi viết lệnh `malloc`, phải trả lời được câu hỏi: *"Hàm nào, hoặc module nào sẽ chịu trách nhiệm gọi `free` cho khối nhớ này?"*
* **Tạo khối thu dọn tập trung (Single Cleanup Block):** Sử dụng cấu trúc nhãn `goto out;` ở cuối hàm để mọi nhánh rẽ xử lý lỗi hay thành công đều đi qua một nơi duy nhất để giải phóng tài nguyên.
* **Xóa con trỏ sau khi free:** Luôn gán `ptr = NULL;` sau khi `free(ptr);` để tránh các lỗi dùng lại con trỏ (`use-after-free`) hoặc giải phóng trùng lặp (`double-free`).

### Việc KHÔNG NÊN làm (What we should NOT do):
* Không lưu trữ các danh sách con trỏ Heap toàn cục bừa bãi mà không có kế hoạch giải phóng rõ ràng.
* **Tuyệt đối không gọi `malloc` trong các vòng lặp thắt nút (Tight Loops)** như `while(1)` mà không giải phóng ngay bên trong hoặc ngay sau vòng lặp đó.
* Không được phớt lờ kết quả trả về `NULL` của `malloc`.

---

## 4. Ví dụ Minh họa bằng Code C

### Sự khác biệt giữa Code lỗi gây rò rỉ bộ nhớ và Code chuẩn sử dụng nhãn Cleanup

```c
#include <stdlib.h>

// Hàm giả định kiểm tra lỗi phần cứng
int error_path_1(void);
int error_path_2(void);

// -------------------------------------------------------------
// TRƯỜNG HỢP XẤU (BAD): Gây rò rỉ bộ nhớ khi gặp lỗi số 2
// -------------------------------------------------------------
int handle_request_bad(void) {
    char *buf = malloc(128);
    if (buf == NULL) return -1;

    if (error_path_1()) {
        free(buf); // Nhánh này có free -> An toàn
        return -1;
    }

    if (error_path_2()) {
        return -1; // LỖI: return sớm nhưng buf không được free -> RÒ RỈ BỘ NHỚ!
    }

    free(buf);
    return 0;
}

// -------------------------------------------------------------
// TRƯỜNG HỢP TỐT (BETTER): Thiết kế một lối thoát duy nhất (Single Exit)
// -------------------------------------------------------------
int handle_request_good(void) {
    char *buf = NULL;
    int rc = -1; // Mặc định trả về lỗi nếu có sự cố xảy ra

    buf = malloc(128);
    if (buf == NULL) goto out; // Nhảy thẳng tới nhãn thu dọn tài nguyên

    if (error_path_1()) goto out;
    if (error_path_2()) goto out;

    rc = 0;  // Vượt qua tất cả các bộ lọc lỗi -> Xác nhận thành công

out:
    free(buf);   // Lệnh free() ở đây cực kỳ an toàn, ngay cả khi buf là NULL
    return rc;   // Trả về mã kết quả cuối cùng
}

```

Trên môi trường mô phỏng Máy tính (Desktop/Host):

Nếu code nhúng của bạn có thể biên dịch và chạy thử trên Linux hoặc Windows (chạy phần logic), hãy tận dụng các công cụ mạnh mẽ sau:

    Valgrind (Memcheck): Chạy chương trình qua Valgrind, nó sẽ thống kê chi tiết dung lượng bộ nhớ bị rò rỉ nằm ở dòng code nào sau khi tắt chương trình.

    AddressSanitizer (ASan): Thêm flag -fsanitize=address khi biên dịch bằng GCC/Clang để phát hiện rò rỉ bộ nhớ ngay lập tức lúc runtime.

Trên Phần cứng Vi điều khiển thực tế (Embedded Target):

Do MCU không có hệ điều hành lớn để chạy Valgrind, bạn phải tự xây dựng cơ chế giám sát thủ công (Heap Accounting):

    Theo dõi lượng RAM trống: Định kỳ sử dụng các hàm kiểm tra của hệ thống (ví dụ xPortGetFreeHeapSize() trong FreeRTOS) và gửi dữ liệu qua UART để vẽ biểu đồ line-graph. Nếu đồ thị lượng RAM trống có xu hướng dốc xuống liên tục theo thời gian, chắc chắn hệ thống đang bị rò rỉ bộ nhớ.

    Kiểm tra Baseline trong chu kỳ Test: Khi chạy thử nghiệm, ép hệ thống thực hiện một tác vụ lặp đi lặp lại 1000 lần (ví dụ nhận/gửi gói tin). Kiểm tra xem sau khi kết thúc 1000 lần đó, lượng RAM trống có quay trở về đúng vạch xuất phát ban đầu hay không. Nếu RAM hụt đi, hãy kiểm tra lại các hàm xử lý gói tin.

    Thêm các cảnh báo (Asserts): Đặt các hàm assert() hoặc phát tín hiệu LED cảnh báo bất cứ khi nào lệnh malloc trả về NULL để phát hiện lỗi sớm trước khi hệ thống rơi vào trạng thái tê liệt.
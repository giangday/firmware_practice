# Chương: Bộ Bảo vệ Bộ nhớ (MPU - Memory Protection Unit) trong Lập trình Nhúng

## 1. MPU là gì? (What it is)
Bộ bảo vệ bộ nhớ (MPU) là một **khối phần cứng vật lý** tích hợp bên trong CPU vi điều khiển, cho phép phân chia không gian bộ nhớ RAM/Flash thành các phân vùng (Regions) độc lập và **giám sát quyền truy cập** của hệ thống đối với từng phân vùng đó.

Đối với mỗi phân vùng được cấu hình trong MPU, bạn có thể thiết lập các quyền hạn khắt khe bao gồm:
* **Quyền Đọc/Ghi (Read/Write access):** Chỉ đọc (Read-only), Đọc và Ghi (Read/Write), hoặc Cấm hoàn toàn (No Access).
* **Quyền thực thi mã lệnh (Execute Never - XN):** Cho phép hoặc ngăn chặn CPU nạp và chạy các lệnh trực tiếp từ vùng nhớ này (chống hack/bảo mật).
* **Quyền đặc quyền (Privilege levels):** Chỉ cho phép hệ điều hành (Kernel/OS ở Privileged mode) truy cập, cấm các tác vụ người dùng thông thường (User tasks ở Unprivileged mode) đụng vào.



---

## 2. Tại sao cần MPU trong Hệ thống Nhúng? (Uses)

Nếu không có MPU, mọi dòng code chạy trên vi điều khiển đều có quyền lực tuyệt đối – một lỗi con trỏ "lạc đề" trong tác vụ giải trí có thể vô tình ghi đè thẳng vào vùng nhớ hệ thống, làm sập toàn bộ thiết bị. MPU sinh ra để đóng vai trò làm "cảnh sát trưởng" kiểm soát biên giới:

* **Bảo vệ Hệ điều hành (Protect Kernel/OS):** Cách ly lõi RTOS (các cấu trúc dữ liệu quản lý, hàng đợi, danh sách task) khỏi các Task người dùng. Nếu một Task bị lỗi, nó không thể làm hỏng hệ điều hành.
* **Cô lập các Tác vụ (Task Isolation):** Trong các ứng dụng phức tạp, MPU có thể đảm bảo Task A không thể đọc hay ghi vào vùng RAM của Task B, ngăn chặn việc rò rỉ dữ liệu hoặc can thiệp chéo lẫn nhau.
* **Bẫy lỗi Tràn Stack lập tức (Catch Stack Overflow):** Bằng cách đặt một phân vùng MPU siêu nhỏ (ví dụ 32 bytes) mang thuộc tính "Cấm truy cập" (No Access) nằm ngay dưới đáy Stack. Khi Stack bị phình to và vừa chạm vào vùng bảo vệ này, phần cứng MPU lập tức phát hiện và kích hoạt ngắt ngoại lệ hệ thống để xử lý, thay vì để Stack âm thầm phá hoại RAM xung quanh.
* **Khóa các vùng dữ liệu cố định (Read-only Regions):** Ép các phân vùng chứa mã lệnh chương trình (Vùng `.text` trên Flash) hoặc mảng hằng số cấu hình luôn ở trạng thái chỉ đọc, chặn đứng mọi nguy cơ tự sửa đổi mã nguồn trái phép (Self-modifying code).



---

## 3. Bản chất Cơ chế Hiện thực (Implementation)

Để MPU hoạt động, phần mềm bắt buộc phải thiết lập cấu hình cho nó ngay trong giai đoạn khởi động hệ thống (`Startup`) hoặc giai đoạn khởi tạo Hệ điều hành:

1. **Định nghĩa Phân vùng:** Khai báo Địa chỉ cơ sở (Base Address) và Kích thước (Size) của phân vùng cần bảo vệ (Lưu ý: Kích thước phân vùng MPU trên một số lõi ARM cũ bắt buộc phải là lũy thừa của 2 và địa chỉ cơ sở phải chia hết cho kích thước đó).
2. **Gán thuộc tính:** Điền các thông số quyền hạn (Access Permissions) vào các thanh ghi chức năng của MPU.
3. **Kích hoạt MPU:** Bật bit cho phép MPU hoạt động tổng thể.

> **Nếu không có MPU:** Hệ thống chỉ có thể dựa vào các bộ kiểm tra logic bằng phần mềm (`software checks`), vốn tiêu tốn rất nhiều chu kỳ CPU để kiểm soát và hoàn toàn bất lực nếu con trỏ bị lỗi nhảy loạn xạ (HardFault).

---

## 4. Ví dụ Minh họa bằng Code C (Cấu hình MPU trên lõi ARM Cortex-M)

Dưới đây là mã nguồn minh họa cách thiết lập một vùng RAM thành "Chỉ đọc ở chế độ Người dùng" bằng thư viện CMSIS tiêu chuẩn:

```c
#include <stdint.h>
// Thư viện CMSIS core của vi điều khiển (ví dụ STM32, SAMD...)
#include "core_cm4.h" 

void MPU_Config_ReadOnly_Region(void) {
    // 1. Tắt MPU trước khi cấu hình lại các thanh ghi
    MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;

    // 2. Chọn phân vùng số 0 (Dòng chip Cortex-M4 thường hỗ trợ 8 phân vùng từ 0-7)
    MPU->RNR = 0;

    // 3. Thiết lập địa chỉ gốc của vùng RAM cần bảo vệ
    // Giả sử bảo vệ vùng nhớ 1KB bắt đầu từ địa chỉ 0x20001000
    MPU->RBAR = 0x20001000;

    // 4. Cấu hình kích thước và quyền hạn truy cập cho phân vùng
    MPU->RASR = (0x00 << MPU_RASR_XN_Pos)   | // XN = 0: Cho phép thực thi mã lệnh từ vùng này
                (0x02 << MPU_RASR_AP_Pos)   | // AP = 010: Hệ điều hành được Đọc/Ghi, nhưng User chỉ được ĐỌC (Read-only)
                (0x09 << MPU_RASR_SIZE_Pos) | // SIZE = 0x09: Tương đương kích thước 1KB (2^(9+1) = 1024)
                (1u   << MPU_RASR_ENABLE_Pos); // Bật riêng phân vùng số 0 này lên

    // 5. Kích hoạt lại MPU tổng thể, cho phép bảo vệ cả trong chế độ Ngắt (HardFault)
    MPU->CTRL |= MPU_CTRL_ENABLE_Msk | MPU_CTRL_HFNMIENA_Msk;
}

```


5. Khi nào nên áp dụng? (Usage)

    Thiết kế hệ thống nhúng có sử dụng RTOS để phân tách quyền lực giữa Kernel và ứng dụng.

    Các sản phẩm thuộc nhóm Safety-critical Systems (Thiết bị y tế, Phanh ô tô, Hệ thống điều khiển bay) đòi hỏi tiêu chuẩn an toàn IEC 61508 / ISO 26262.

    Cơ chế Secure Boot: Khóa phân vùng chứa mã nạp và bộ sinh khóa mật mã học (Cryptographic keys) ngay sau khi khởi động xong để chống các cuộc tấn công chiếm quyền điều khiển phần cứng.

6. Câu hỏi Thực tế (Practical Questions)
Q1: Điều gì xảy ra nếu một Task cố tình ghi dữ liệu vào một phân vùng đã được MPU cấu hình là Chỉ đọc (Read-only)?

    Trả lời: Ngay lập tức tại thời điểm CPU thực hiện lệnh ghi đó, phần cứng MPU sẽ chặn đứng hành vi này lại (Dữ liệu sai trái không bao giờ được ghi xuống ô nhớ RAM thực tế). Đồng thời, MPU sẽ kích hoạt một lỗi ngoại lệ phần cứng khẩn cấp của hệ thống, thường gọi là MemManage Fault (Memory Management Fault) hoặc HardFault trên lõi ARM. Hệ điều hành sẽ nhảy vào hàm ngắt lỗi này, tại đây nó có quyền hủy (terminate) riêng Task vi phạm đó để giải phóng tài nguyên và giữ cho các Task quan trọng khác tiếp tục hoạt động bình thường mà không cần phải reset lại toàn bộ con chip.

Q2: Tại sao người ta nói MPU là một thành phần bắt buộc để đạt được các chứng chỉ an toàn (Safety Certificates) cho thiết bị y tế hay công nghiệp?

    Trả lời: Vì phần mềm dù có được test kỹ đến đâu vẫn luôn có xác suất xảy ra lỗi ngoài ý muốn lúc runtime (ví dụ: nhiễu điện từ làm lật bit của con trỏ, lỗi tràn bộ đệm chưa được phát hiện). Trong các thiết bị y tế (như máy trợ tim) hay công nghiệp (như robot tự hành), một lỗi phần mềm làm treo toàn bộ chip có thể đánh đổi bằng sinh mạng con người. Chứng chỉ an toàn yêu cầu hệ thống phải có khả năng Tự bao vây lỗi (Fault Containment). MPU chính là công cụ duy nhất bằng phần cứng giúp bạn chứng minh với các tổ chức kiểm định rằng: "Ngay cả khi phần mềm chạy tác vụ hiển thị màn hình của tôi bị crash hoặc bị tấn công, lỗi đó cũng bị cô lập hoàn toàn và không cách nào lan sang phá hỏng phân vùng điều khiển nhịp tim hay phanh khẩn cấp được".
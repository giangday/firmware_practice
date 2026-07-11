# Chương: Tràn bộ nhớ Stack (Stack Overflow) trong Lập trình Nhúng

## 1. Stack Overflow là gì? (What it is)
Tràn bộ nhớ Stack là hiện tượng vùng nhớ **Stack bị phình to vượt quá giới hạn** dung lượng được định sẵn cho nó trong file cấu hình linker (`.ld` hoặc `.icf`). 

Trong kiến trúc bộ nhớ của vi điều khiển, Stack và Heap thường được bố trí đối đỉnh và phát triển ngược chiều nhau (hoặc Stack nằm ngay sát phân vùng dữ liệu khác). Khi Stack tăng trưởng quá mức, nó sẽ **xâm lấn và ghi đè** lên phân vùng của Heap, các biến toàn cục (vùng `.bss`, `.data`), hoặc thậm chí là vượt quá ranh giới vật lý của RAM.



---

## 2. Nguyên nhân gây tràn Stack trong Hệ thống Nhúng

Có 3 "thủ phạm" chính thường trực gây ra lỗi này:

1. **Khai báo mảng cục bộ quá lớn (Large Local Arrays):** Mọi biến cục bộ khai báo bên trong một hàm (ví dụ: `uint8_t buffer[1024];`) đều lấy tài nguyên từ Stack. Trên các dòng chip nhỏ có RAM chỉ từ vài KB đến vài chục KB, một mảng vài KB sẽ lập tức bóp nghẹt Stack.
2. **Hàm đệ quy (Recursion):** Khi một hàm tự gọi lại chính nó, mỗi tầng gọi hàm sẽ PUSH một loạt thông tin (địa chỉ quay về, các tham số, các biến cục bộ) vào Stack. Nếu không có điều kiện dừng hoặc đệ quy quá sâu, Stack sẽ cạn kiệt rất nhanh.
3. **Chuỗi gọi hàm quá sâu (Deep Call Chains):** Hàm `A` gọi hàm `B`, hàm `B` gọi hàm `C`... nối tiếp nhau qua hàng chục tầng. Kịch bản này càng nguy hiểm hơn nếu một hàm ngắt (ISR) xảy ra ở tầng sâu nhất, vì ISR sẽ tiếp tục PUSH dữ liệu đè lên đỉnh Stack hiện tại.

---

## 3. Cách phòng tránh và Phòng ngự (Prevention)

Để bảo vệ hệ thống khỏi thảm họa tràn Stack, các kỹ sư nhúng áp dụng các chiến lược sau:

* **Tuyệt đối tránh hoặc hạn chế tối đa đệ quy:** Thay thế các thuật toán đệ quy bằng các vòng lặp (`for`, `while`) thông thường (Iterative approach).
* **Không dùng mảng cục bộ lớn:** Di chuyển các mảng lớn ra ngoài hàm để biến chúng thành bộ nhớ tĩnh (`static`) hoặc cấp phát trên Heap (nếu hệ thống cho phép dùng Heap).
* **Ước tính dung lượng Stack tệ nhất (Worst-case Stack Usage):** * *Phân tích tĩnh (Static Analysis):* Sử dụng công cụ của trình biên dịch (ví dụ thuộc tính `-fstack-usage` trong GCC) để tính toán độ sâu tối đa của Stack dựa trên cấu trúc gọi hàm.
  * *Kỹ thuật tô màu Stack (Stack Paint / Fill Pattern):* Lúc khởi động, ghi toàn bộ vùng nhớ Stack bằng một giá trị đặc biệt (ví dụ `0xCDCDCDCD`). Sau khi hệ thống chạy một thời gian, dùng công cụ Debug để quét xem giá trị `0xCDCD` còn lại bao nhiêu, từ đó biết được Stack đã "lội" sâu đến mức nào.
* **Sử dụng cơ chế phần cứng (MPU / Stack Guards):** Cấu hình Bộ bảo vệ bộ nhớ (MPU - Memory Protection Unit) để tạo ra một vùng đệm "cấm" (Guard region) ở đáy Stack. Nếu Stack chớm chạm vào vùng này, phần cứng sẽ lập tức kích hoạt ngắt ngoại lệ (`HardFault` hoặc `MemManage`) để hệ thống xử lý thay vì để nó âm thầm ghi đè làm hỏng dữ liệu khác.

---

## 4. Ví dụ Minh họa bằng Code C

### Sự khác biệt giữa code gây nguy hiểm và code an toàn

```c
#include <stdint.h>

// -------------------------------------------------------------
// TRƯỜNG HỢP XẤU (Risky): Cấp phát mảng lớn trên Stack
// -------------------------------------------------------------
void process_network_data_bad(void) {
    // Mảng 4096 bytes này nằm trên Stack. 
    // Trên một MCU nhỏ (ví dụ RAM 8KB, Stack 1KB), hàm này gọi lên sẽ gây crash lập tức.
    char buf[4096]; 
    
    // Thực hiện xử lý dữ liệu với buf...
}

// -------------------------------------------------------------
// TRƯỜNG HỢP TỐT HƠN (Safer): Đổi sang Bộ nhớ Tĩnh cố định
// -------------------------------------------------------------
// Mảng này được đẩy sang phân vùng .bss trong RAM từ lúc biên dịch.
// Dung lượng Stack tiêu tốn cho hàm better() lúc này gần như bằng 0.
static char safe_buf[4096]; 

void process_network_data_better(void) {
    // Sử dụng trực tiếp bộ đệm tĩnh safe_buf
    // safe_buf[0] = ...
}
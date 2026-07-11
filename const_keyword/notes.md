# Chương: Từ khóa `const` trong Lập trình Nhúng

## 1. Tổng quan về `const` (What it does)
Từ khóa `const` (viết tắt của *constant*) được sử dụng để tuyên bố rằng một biến hoặc vùng dữ liệu được trỏ tới là **Chỉ đọc (Read-only)**. 
* Đây là một lời cam kết với trình biên dịch (compiler). 
* Nếu có bất kỳ đoạn code nào cố tình ghi đè hoặc thay đổi giá trị của biến `const` sau khi khởi tạo, trình biên dịch sẽ chặn lại và báo lỗi ngay lập tức (**Compile-time error**).

---

## 2. Tầm quan trọng trong Hệ thống Nhúng (Why it matters)

Trong lập trình nhúng, `const` không chỉ giúp code an toàn hơn mà còn là công cụ tối ưu hóa phần cứng cốt lõi nhờ vào kiến trúc bộ nhớ vi điều khiển (gồm RAM và Flash/ROM):

* **Tối ưu hóa bộ nhớ (Optimization):** Mặc định, các biến toàn cục thông thường sẽ được lưu tại RAM để có thể đọc/ghi. Tuy nhiên, RAM của vi điều khiển thường rất nhỏ (vài KB đến vài trăm KB). Khi thêm `const` vào các mảng dữ liệu lớn, trình biên dịch sẽ đẩy thẳng dữ liệu này vào **Flash/ROM** (vùng nhớ chỉ đọc, dung lượng lớn). Điều này giúp tiết kiệm dung lượng RAM quý giá bằng **0 bytes** tiêu tốn cho mảng đó.
* **Tính chính xác & An toàn (Correctness):** Ngăn chặn các lỗi nghiêm trọng do con trỏ "lạc đề" vô tình ghi đè lên các vùng dữ liệu cố định quan trọng (như bảng tra cứu cấu hình, ma trận hàm toán học).
* **Tài liệu hóa mã nguồn (Documentation):** Làm rõ thiết kế của API. Nhìn vào tham số hàm có `const`, người phát triển biết ngay hàm đó an toàn và không thay đổi dữ liệu gốc.

---

## 3. Bản chất các biến thể Con trỏ `const` (Pointer Placement)

Vị trí của từ khóa `const` đứng trước hay sau dấu sao `*` sẽ quyết định **đối tượng nào bị khóa**:

| Cú pháp | Loại | Bản chất | Ví dụ hợp lệ / lỗi |
| :--- | :--- | :--- | :--- |
| `const int *p`<br>*(hoặc `int const *p`)* | **Data const** | **Dữ liệu** bị khóa (Read-only data). Không thể sửa giá trị thông qua con trỏ, nhưng có thể đổi địa chỉ mà con trỏ trỏ tới. | `*p = 10;` ❌ *Lỗi*<br>`p = &other_var;`  *Hợp lệ* |
| `int *const p` | **Pointer const** | **Địa chỉ** bị khóa (Read-only pointer). Con trỏ cố định vào một địa chỉ, không thể trỏ đi nơi khác, nhưng có thể sửa dữ liệu bên trong. | `*p = 10;`  *Hợp lệ*<br>`p = &other_var;` ❌ *Lỗi* |
| `const int *const p` | **Cả hai đều const** | **Khóa toàn diện**. Cấm thay đổi cả dữ liệu lẫn địa chỉ trỏ của con trỏ. | `*p = 10;` ❌ *Lỗi*<br>`p = &other_var;` ❌ *Lỗi* |

---

## 4. Kết hợp `const` với các Qualifiers khác

### `const volatile` (hoặc `volatile const`)
* **Ý nghĩa:** Giá trị của biến **có thể thay đổi bất ngờ** do tác động từ bên ngoài (phần cứng hoặc hàm ngắt ISR), nhưng **đoạn code hiện tại không được phép ghi dữ liệu vào**.
* **Ứng dụng:** Sử dụng cho các thanh ghi chỉ đọc (Read-only Hardware Registers) như thanh ghi trạng thái (Status Register) hoặc thanh ghi dữ liệu đầu vào (Input Data Register - GPIOx_IDR).

### `static const`
* **Ý nghĩa:** Giới hạn phạm vi truy cập của hằng số chỉ trong nội bộ file hiện tại (`static` - internal linkage) và lưu trữ nó hoàn toàn trong bộ nhớ **Flash/ROM** (`const`).
* **Ứng dụng:** Thường dùng cho bảng tra cứu dữ liệu (Lookup tables), font chữ hiển thị, hoặc mảng cấu hình cố định của một module.

---

## 5. Ví dụ Minh họa bằng Code C

```c
#include <stdint.h>

// 1. Mảng dữ liệu lớn được lưu trong Flash/ROM thay vì RAM
static const uint8_t Lookup_Table[256] = { 0x00, 0x01, 0x02, /* ... */ 0xFF };

// 2. Ứng dụng const volatile cho thanh ghi đầu vào (Chỉ đọc)
#define REG_INPUT_STATUS  (*(volatile const uint32_t *)0x40020010)

// 3. Ứng dụng const trong tham số hàm (Bảo vệ chuỗi truyền vào)
void UART_SendString(const char *str) {
    while (*str) {
        // REG_UART_DATA = *str; // Chỉ đọc dữ liệu từ *str
        str++; // Hợp lệ, vì bản thân con trỏ str không bị cố định địa chỉ
    }
}

int main(void) {
    int x = 10;
    int y = 20;

    const int *p1 = &x;     // Data const
    int *const p2 = &x;     // Pointer const

    // p1 = &y;             // Hợp lệ
    // *p1 = 5;             // LỖI BIÊN DỊCH!

    // *p2 = 5;             // Hợp lệ
    // p2 = &y;             // LỖI BIÊN DỊCH!

    return 0;
}
```
---
## 6. Câu hỏi

### Q1: Khi nào tôi nên để tham số của hàm là const?
- Luôn luôn sử dụng const cho tham số của hàm khi bạn truyền dữ liệu theo kiểu Con trỏ (Pointer) hoặc Mảng (Array) sang một hàm mục đích chỉ để đọc (ví dụ: void Display_Buffer(const uint8_t *buf, uint16_t len);). 
- Việc này ngăn hàm phát triển sau này vô tình làm thay đổi hay làm hỏng dữ liệu gốc của caller truyền vào.

### Q2: Từ khóa const giúp giữ các bảng dữ liệu lớn trong Flash/ROM của vi điều khiển như thế nào?
- Khi biên dịch, trình biên dịch phân tích các biến toàn cục hoặc biến static. Nếu không có const, hệ thống bắt buộc phải cấp phát RAM và thực hiện copy dữ liệu từ Flash sang RAM khi khởi động (trong file startup.s) để CPU có thể sửa đổi khi chạy. Khi có const, trình biên dịch biết dữ liệu này là bất biến, nó sẽ không cấp phát RAM nữa mà cấu hình để CPU đọc trực tiếp từ địa chỉ trên Flash/ROM mỗi khi cần dùng.
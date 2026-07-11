#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdint>

using namespace std;


//Q1: Làm thế nào để SET (bật thành 1) và CLEAR (xóa về 0) một bit trong biến status?
void q1(){
    uint8_t status = 0;
    const uint8_t BIT_MASK = (1u <<3);
    status |= BIT_MASK; // SET bit thứ 3
    cout << "Status after SET bit 3: " << bitset<8>(status) << endl;
    status &= ~BIT_MASK; // CLEAR bit thứ 3
    cout << "Status after CLEAR bit 3: " << bitset<8>(status) << endl;
}
//Q2: Làm thế nào để KIỂM TRA (test) xem bit N đang là 0 hay 1?
void q2(){
    uint8_t flag = 0x14;
    uint8_t n = 3;

    if(flag & (1u << n)) {
        cout << "Bit " << (int)n << " is set to 1" << endl;
    } else {
        cout << "Bit " << (int)n << " is set to 0" << endl;
    }
}
//Q3: Làm thế nào để GỘP hai giá trị 8-bit thành một biến 16-bit?
void q3(){
    uint8_t high = 0x12;
    uint8_t low = 0x34;

    uint16_t combined = (uint16_t)high <<8 | low;
    cout << "Combined 16-bit value: " << hex << combined << endl;
    
}
//Q4: Trong một biến 16-bit, làm sao để chỉ LẤY 12 bit đầu và BỎ 4 bit cuối?
void q4(){
    uint16_t BIT_MASK = 0xFFF0;
    uint16_t value = 0xABCD;
    uint16_t result = value & BIT_MASK;
    cout << "Result after masking: " << hex << result << endl;
}

//Q5: Làm thế nào để TÁCH một biến 16-bit thành hai biến 8-bit?
void q5(){
    uint16_t combined = 0xABCD;
    uint8_t high = combined >> 8;
    uint8_t low = combined & 0xFF;
    cout << "High byte: " << hex << (int)high << endl;
    cout << "Low byte: " << hex << (int)low << endl; 
}

//Q6: Chúng ta có hai biến 8-bit; làm thế nào để đọc một giá trị duy nhất từ 12 bit cao (Upper 12 bits)?
void q6(){
    uint8_t high = 0xAB;
    uint8_t low = 0xCD;
    uint16_t combined = ((uint16_t) high << 8 | low) >> 4;
    cout << "Upper 12 bits value: " << hex << (int)combined << endl;
}

int main() {
    q6();
    return 0;
}
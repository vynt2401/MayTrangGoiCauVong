#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "sl_udelay.h"
#include "sl_iostream.h"
#include "sl_iostream_init_instances.h"
#include "sl_iostream_handles.h"
#include <stdio.h>

// --- CẤU HÌNH CHÂN DATA DHT11 ---
// Lưu ý: Đảm bảo bạn đang nối đúng chân PC0
#define DHT_PORT gpioPortC
#define DHT_PIN  0

// Giới hạn vòng lặp để tránh treo chip nếu tuột dây (Anti-hang)
#define MAX_TIMEOUT 10000

// Biến toàn cục lưu dữ liệu
uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
uint16_t SUM;

// --- HÀM GỬI TÍN HIỆU START ---
void DHT11_Start(void)
{
    // Cấu hình chân là Output
    GPIO_PinModeSet(DHT_PORT, DHT_PIN, gpioModePushPull, 1);

    // Kéo xuống thấp 20ms (Datasheet yêu cầu tối thiểu 18ms)
    GPIO_PinOutClear(DHT_PORT, DHT_PIN);
    sl_udelay_wait(20000);

    // Kéo lên cao 30us sau đó thả nổi chân (chuyển sang Input)
    GPIO_PinOutSet(DHT_PORT, DHT_PIN);
    sl_udelay_wait(30);

    // Chuyển sang Input để chờ phản hồi từ cảm biến
    GPIO_PinModeSet(DHT_PORT, DHT_PIN, gpioModeInput, 1);
}

// --- HÀM KIỂM TRA PHẢN HỒI (CÓ TIMEOUT) ---
uint8_t DHT11_Check_Response(void)
{
    uint32_t timeout = 0;

    // 1. Chờ chân DHT xuống thấp (Response bắt đầu)
    // Nếu chờ quá lâu mà chân vẫn cao -> Lỗi dây hoặc hỏng sensor
    while (GPIO_PinInGet(DHT_PORT, DHT_PIN) == 1)
    {
        timeout++;
        if (timeout > MAX_TIMEOUT) return 0; // Lỗi: Không thấy phản hồi
    }

    // 2. Chờ chân DHT hết mức thấp (80us)
    timeout = 0;
    while (GPIO_PinInGet(DHT_PORT, DHT_PIN) == 0)
    {
        timeout++;
        if (timeout > MAX_TIMEOUT) return 0; // Lỗi: Kẹt ở mức thấp
    }

    // 3. Chờ chân DHT hết mức cao (80us) để bắt đầu truyền bit đầu tiên
    timeout = 0;
    while (GPIO_PinInGet(DHT_PORT, DHT_PIN) == 1)
    {
        timeout++;
        if (timeout > MAX_TIMEOUT) return 0; // Lỗi: Kẹt ở mức cao
    }

    return 1; // Phản hồi OK
}

// --- HÀM ĐỌC 1 BYTE (ĐÃ SỬA LỖI KHỞI TẠO) ---
uint8_t DHT11_Read(void)
{
    uint8_t i = 0, j; // QUAN TRỌNG: Phải khởi tạo i = 0
    uint32_t timeout;

    for (j = 0; j < 8; j++)
    {
        // A. Chờ hết khoảng 50us mức thấp trước mỗi bit
        timeout = 0;
        while (GPIO_PinInGet(DHT_PORT, DHT_PIN) == 0)
        {
            timeout++;
            if (timeout > MAX_TIMEOUT) return 0; // Thoát nếu treo
        }

        // B. Phân biệt bit 0 hay 1
        // Bit 0: Mức cao 26-28us
        // Bit 1: Mức cao 70us
        // -> Ta đợi khoảng 40us rồi kiểm tra.
        sl_udelay_wait(40);

        if (GPIO_PinInGet(DHT_PORT, DHT_PIN) == 0)
        {
            // Nếu chân đã xuống thấp -> Tín hiệu mức cao ngắn -> Bit 0
            // i ban đầu đã là 0, nên không cần làm gì
        }
        else
        {
            // Nếu chân vẫn cao -> Tín hiệu mức cao dài -> Bit 1
            i |= (1 << (7 - j)); // Ghi bit 1 vào vị trí tương ứng

            // Chờ nốt cho hết mức cao của bit 1 này
            timeout = 0;
            while (GPIO_PinInGet(DHT_PORT, DHT_PIN) == 1)
            {
                timeout++;
                if (timeout > MAX_TIMEOUT) return 0; // Thoát nếu treo
            }
        }
    }
    return i;
}

// --- INIT ---
void app_init(void)
{
    // In thông báo khởi động
    printf("\r\n--- KHOI DONG HE THONG ---\r\n");
    sl_udelay_wait(1000000); // Đợi 1s cho điện áp ổn định
}

// --- MAIN LOOP ---
void app_process_action(void)
{
    printf("Dang doc DHT11... ");

    DHT11_Start();

    if (DHT11_Check_Response() == 1)
    {
        // Đọc dữ liệu
        Rh_byte1   = DHT11_Read(); // Độ ẩm phần nguyên
        Rh_byte2   = DHT11_Read(); // Độ ẩm phần thập phân
        Temp_byte1 = DHT11_Read(); // Nhiệt độ phần nguyên
        Temp_byte2 = DHT11_Read(); // Nhiệt độ phần thập phân
        SUM        = DHT11_Read(); // Checksum

        // Tính toán Checksum: Tổng 4 byte đầu phải bằng byte cuối
        if (SUM == (uint8_t)(Rh_byte1 + Rh_byte2 + Temp_byte1 + Temp_byte2))
        {
            printf("[OK] Do am: %d.%d %% | Nhiet do: %d.%d C\r\n",
                   Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2);
        }
        else
        {
            // In ra giá trị raw để debug nếu sai checksum
            printf("[LOI] Sai Checksum! (Tinh: %d, Nhan: %d)\r\n",
                   (Rh_byte1 + Rh_byte2 + Temp_byte1 + Temp_byte2), SUM);
        }
    }
    else
    {
        printf("[LOI] Khong tim thay DHT11 (Check day noi!)\r\n");
    }

    // Đợi 2 giây (2.000.000 us) trước khi đo lại
    sl_udelay_wait(2000000);
}

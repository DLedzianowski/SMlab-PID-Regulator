# STM32 Temperature Controller  

Regulator temperatury oparty na mikrokontrolerze STM32, obsługujący czujnik BME280, wyświetlacz OLED SSD1306 oraz tranzystor IRF520 do sterowania grzałką. Projekt zawiera implementację regulatora PID z funkcją anty-windup.  

## Funkcje projektu  
- **Pomiar temperatury**: Czujnik BME280 podłączony przez I2C.  
- **Wyświetlanie danych**: Ekran OLED 0.91" SSD1306 obsługiwany przez I2C.  
- **Regulator PID**: Obsługuje algorytmy PI i PID z anty-windup.  
- **Sterowanie grzałką**: Tranzystor IRF520 sterowany sygnałem PWM.  
- **Interfejs użytkownika**: Zmiana i zatwierdzanie wartości zadanej przez enkoder mechaniczny.  
- **Bezpieczeństwo**: Automatyczne wyłączanie grzałki powyżej 60°C.  
- **Komunikacja UART**: Zmiana wartości zadanej i monitorowanie danych systemowych.  

## Sprzęt użyty w projekcie  
- **Płytka rozwojowa**: NUCLEO-F746ZG  
- **Wbudowany User Button**: Nawigacja między ekranami  
- **Enkoder mechaniczny**: Obsługa zmiany wartości zadanej  
- **Rezystor**: 10Ω, 10W jako element grzałki  
- **Zasilacz**: 12V z ograniczeniem prądowym (800mA)  

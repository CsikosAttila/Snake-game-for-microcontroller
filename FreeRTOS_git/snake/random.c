#include "random.h"
#include "em_device.h"
#include "em_chip.h"
#include "em_adc.h"

#define RANDOM_SEQUENCE_LENGTH 37

/// Represents a sequence of randomly sorted segments of the field
Segment randomSequence[RANDOM_SEQUENCE_LENGTH] = {
#include "random_values.txt"
};

uint16_t getRandomNumber(void)
{
    uint16_t adc_result=0; // belso valtozo ami a random szam lesz
    ADC0->CMD = 0x1; // Elinditjuk a mintavetelezest

    while(!(ADC0->STATUS & (1 << 16))); // Megvarjuk, hogy stabilizalodjon az eredmeny
    adc_result = ADC0->SINGLEDATA; // Kiolvassuk az eredmenyt

    return adc_result;
}

Segment getRandomSegment()
{
    int rand = getRandomNumber() % RANDOM_SEQUENCE_LENGTH;
    return randomSequence[rand];
}

void initRandom(void)
{
    // Engedelyezzuk az orajelet a z ADC-nek (16. bit)
    CMU->HFPERCLKEN0 |= (1 << 16);

    // Konfiguraljuk az ADC0-t
    // 1. A felmelegedesi idot 24 orajelciklusra allitjuk (16. bitte kezdve), a prescalert pedig 1-re allitjuk (8. bit)
    // 2. Konfigurajuk az egyszeri mintavetelezest, a referencia feszultsegnek VDD-t vesszuk (16. bit), a mintat pedig a 6-os csatornara vesszuk
    // 3. Tiltjuk az ADC0 interruptjat
    ADC0->CTRL = (24 << 16) | (1 << 8);
    ADC0->SINGLECTRL = (2 << 16) | (6 << 8);
    ADC0->IEN = 0x0;
}

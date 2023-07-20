#include "display.h"

#include "../segmentlcd.h"
#include "../segmentlcd_individual.h"

/// The LCD segments of the lower display
SegmentLCD_LowerCharSegments_TypeDef charSegments[SEGMENT_LCD_NUM_OF_LOWER_CHARS];

bool areSegmentsEqual(const Segment segment1, const Segment segment2)
{
    return segment1.row == segment2.row && segment1.column == segment2.column;
}

/// Sets or resets the specified segment
/// @param segment The segment to set or reset
/// @param isSet Whether to set or reset the segment
void setSegment(Segment segment, bool isSet) {
    switch (segment.row)
    {
    case 0:
        charSegments[segment.column].a = isSet;
        break;

    case 1:
        if (segment.column == SEGMENT_LCD_NUM_OF_LOWER_CHARS) // handle the last column as a special case, as the right segments of the last digit
            charSegments[SEGMENT_LCD_NUM_OF_LOWER_CHARS - 1].b = isSet;
        else
            charSegments[segment.column].f = isSet;

        break;

    case 2:
        charSegments[segment.column].g = charSegments[segment.column].m = isSet;
        break;

    case 3:
        if (segment.column == SEGMENT_LCD_NUM_OF_LOWER_CHARS) // handle the last column as a special case, as the right segments of the last digit
            charSegments[SEGMENT_LCD_NUM_OF_LOWER_CHARS - 1].c = isSet;
        else
            charSegments[segment.column].e = isSet;
        break;

    case 4:
        charSegments[segment.column].d = isSet;
        break;
    
    default:
        break;
    }
}

/// Resetes all segments 
void resetAllSegments() {
    for (uint8_t digit = 0; digit < SEGMENT_LCD_NUM_OF_LOWER_CHARS; ++digit)
        charSegments[digit].raw = 0;
}

void displaySegmentList(Segment segments[], int segmentsCount, bool reset) {
    if (reset)
        resetAllSegments(); // reset all segments in the charSegments array

    for (int i = 0; i < segmentsCount; ++i)
        setSegment(segments[i], /* isSet = */ true); // set each segment specified in the segments array

    SegmentLCD_LowerSegments(charSegments); // display the segments
}

void displaySegment(Segment segment, bool reset) {
    displaySegmentList(&segment, 1, reset);
}

void displayScore(int score) {
    SegmentLCD_Number(score);
}

void setDots(bool isSet) {
    // set the decimal dots of all digits in the lower digit display
    SegmentLCD_Symbol(LCD_SYMBOL_DP2, isSet);
    SegmentLCD_Symbol(LCD_SYMBOL_DP3, isSet);
    SegmentLCD_Symbol(LCD_SYMBOL_DP4, isSet);
    SegmentLCD_Symbol(LCD_SYMBOL_DP5, isSet);
    SegmentLCD_Symbol(LCD_SYMBOL_DP6, isSet);

    SegmentLCD_Symbol(LCD_SYMBOL_GECKO, isSet); // flash the Gecko symbol along with the dots
}

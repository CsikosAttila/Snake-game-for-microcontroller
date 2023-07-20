#ifndef DISPLAY
#define DISPLAY

#include <stdbool.h>

/// Represents a segment of the grid of the game board
typedef struct {
    int row;
    int column;
} Segment;

/// Gets whether two segments are equal
bool areSegmentsEqual(const Segment segment1, const Segment segment2);

/// Displays the specified segments on the lower display
/// @param segments An array of segments to display
/// @param segmentsCount The number of segments in the segments array
/// @param reset Whether to reset all segments before displaying the specified segments
void displaySegmentList(Segment segments[], int segmentsCount, bool reset);

/// Displays the specified segments
/// @param reset If true, all segments will be reset before displaying the specified segments
void displaySegment(Segment segment, bool reset);

/// Displays the specified score value on the upper display
void displayScore(int score);

/// Enables or disables the decimal dots of all digits in the lower digit display
void setDots(bool isSet);

#endif

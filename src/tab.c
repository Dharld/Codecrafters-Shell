#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#define TAB_KEY 9

// Store original terminal settings
struct termios original_termios;

// Function to restore original terminal settings
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

// Function to enable raw mode
void enableRawMode() {
    // Get current terminal attributes
    tcgetattr(STDIN_FILENO, &original_termios);
    
    // Register cleanup function to restore terminal on exit
    atexit(disableRawMode);
    
    // Create a copy to modify
    struct termios raw = original_termios;
    
    // Modify settings for raw mode:
    // Disable echo (don't show typed characters)
    // Disable canonical mode (read input byte-by-byte instead of line-by-line)
    // Disable Ctrl-C and Ctrl-Z signals
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    
    // Disable software flow control (Ctrl-S and Ctrl-Q)
    // Disable CR to NL translation
    raw.c_iflag &= ~(IXON | ICRNL);
    
    // Set character timeout and minimum input
    raw.c_cc[VMIN] = 1;   // Wait for at least one character
    raw.c_cc[VTIME] = 0;  // No timeout
    
    // Apply the new settings
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Basic test to detect TAB key
void testTabDetection() {
    enableRawMode();
    
    printf("Press keys (TAB to test, Ctrl-C to exit):\n");
    
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == TAB_KEY) {
            printf("\r\nTAB key detected!\r\n");
        } else {
            // Print the character and its ASCII value
            printf("\r\nKey pressed: '%c' (%d)\r\n", c, c);
        }
    }
    
    // No need to call disableRawMode() here as it's registered with atexit()
}

int main() {
    testTabDetection();
    return 0;
}

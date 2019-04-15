// Based upon the amazing 2-line character set designed by Michael Pilcher

// Cleaned up for space, efficiency:
//
// - Single array for programmable characters
// - Looped initialization from this table, rather than a series of calls
// - Big character composition stored in strings, rather than drawn in functions
// One-line per character definition, instead of 11 or so
// Makes character tweaking a lot easier
// - Characters in an ASCII table, to encourage adding of more characters
// - Added the rest of ascii characters 0x20-0x5F (some could use improvement :)
// - writeString() function
// - Far fewer system calls by writing strings rather than one-byte-per-call
// (Note: I changed back to one-byte-per-write, as writing multiple bytes at once
// seems to fail to wrap in the LCD's circular display buffer).
// - Handles wrapping of LCD circular display buffer :)
// - Writes out blank spots in each letter, for cleaner overwriting of previous text
// - Uses Space instead of hex 254 blank character; possibly more portable to other displays
// - The demo loop is far more elegant (no more x = x + 4, x = x + 4, x = x + 4 :)
// - Initialization (begin) called before programming the characters
// (Without this, I had inconsistent character programming between runs.)

// This should run faster, be more stable, provide more characters, take half the space,
// and be easier to maintain, than the original.
//
// Dale Gass
// May 10, 2014

#include <LiquidCrystal_I2C.h>

// Taylor to your pin arrangement
// My wiring keeps the 'interruptable' pins 2/3 free for interrupt sue
// and pins 10/11 free for SPI communictions/PWM. Plus on the mini Pro,
// the six control lines are now sequential along the side of the board,
// making wiring easier (using a pin header).

LiquidCrystal_I2C lcd(0x27,16,2);

/* My circuit:
  LCD RS pin to digital pin 9
  LCD Enable pin to digital pin 8
  LCD D4 pin to digital pin 7
  LCD D5 pin to digital pin 6
  LCD D6 pin to digital pin 5
  LCD D7 pin to digital pin 4
  LCD R/W pin to ground
  10K resistor:
  ends to +5V and ground - I just grounded pin 3 for full contract
  wiper to LCD VO pin (pin 3)
*/

// The standard wiring which prevents int 2/3 and SPI use:
// Most common in Arduino LCD samples
// LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Eight programmable character definitions
byte custom[8][8] = {
  { B11111,
    B11111,
    B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000
  },

  { B11100,
    B11110,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111
  },

  { B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B01111,
    B00111
  },

  { B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111,
    B11111
  },

  { B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11110,
    B11100
  },

  { B11111,
    B11111,
    B11111,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111
  },

  { B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111,
    B11111
  },

  { B00111,
    B01111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111
  }
};

// Characters, each with top and bottom half strings
// \nnn string encoding is octal, so:
// \010 = 8 decimal (8th programmable character)
// \024 = 20 decimal (space)
// \377 = 255 decimal (black square)

const char *bigChars[][2] = {
  {"\024\024\024", "\024\024\024"}, // Space
  {"\377", "\007"}, // !
  {"\005\005", "\024\024"}, // "
  {"\004\377\004\377\004", "\001\377\001\377\001"}, // #
  {"\010\377\006", "\007\377\005"}, // $
  {"\001\024\004\001", "\004\001\024\004"}, // %
  {"\010\006\002\024", "\003\007\002\004"}, // &
  {"\005", "\024"}, // '
  {"\010\001", "\003\004"}, // (
  {"\001\002", "\004\005"}, // )
  {"\001\004\004\001", "\004\001\001\004"}, // *
  {"\004\377\004", "\001\377\001"}, // +
  {"\024", "\005"}, // ,
  {"\004\004\004", "\024\024\024"}, // -
  {"\024", "\004"}, // .
  {"\024\024\004\001", "\004\001\024\024"}, // /
  {"\010\001\002", "\003\004\005"}, // 0
  {"\001\002\024", "\024\377\024"}, // 1
  {"\006\006\002", "\003\007\007"}, // 2
  {"\006\006\002", "\007\007\005"}, // 3
  {"\003\004\002", "\024\024\377"}, // 4
  {"\377\006\006", "\007\007\005"}, // 5
  {"\010\006\006", "\003\007\005"}, // 6
  {"\001\001\002", "\024\010\024"}, // 7
  {"\010\006\002", "\003\007\005"}, // 8
  {"\010\006\002", "\024\024\377"}, // 9
  {"\004", "\001"}, // :
  {"\004", "\005"}, // ;
  {"\024\004\001", "\001\001\004"}, // <
  {"\004\004\004", "\001\001\001"}, // =
  {"\001\004\024", "\004\001\001"}, // >
  {"\001\006\002", "\024\007\024"}, // ?
  {"\010\006\002", "\003\004\004"}, // @
  {"\010\006\002", "\377\024\377"}, // A
  {"\377\006\005", "\377\007\002"}, // B
  {"\010\001\001", "\003\004\004"}, // C
  {"\377\001\002", "\377\004\005"}, // D
  {"\377\006\006", "\377\007\007"}, // E
  {"\377\006\006", "\377\024\024"}, // F
  {"\010\001\001", "\003\004\002"}, // G
  {"\377\004\377", "\377\024\377"}, // H
  {"\001\377\001", "\004\377\004"}, // I
  {"\024\024\377", "\004\004\005"}, // J
  {"\377\004\005", "\377\024\002"}, // K
  {"\377\024\024", "\377\004\004"}, // L
  {"\010\003\005\002", "\377\024\024\377"}, // M
  {"\010\002\024\377", "\377\024\003\005"}, // N
  {"\010\001\002", "\003\004\005"}, // 0/0
  {"\377\006\002", "\377\024\024"}, // P
  {"\010\001\002\024", "\003\004\377\004"}, // Q
  {"\377\006\002", "\377\024\002"}, // R
  {"\010\006\006", "\007\007\005"}, // S
  {"\001\377\001", "\024\377\024"}, // T
  {"\377\024\377", "\003\004\005"}, // U
  {"\003\024\024\005", "\024\002\010\024"}, // V
  {"\377\024\024\377", "\003\010\002\005"}, // W
  {"\003\004\005", "\010\024\002"}, // X
  {"\003\004\005", "\024\377\024"}, // Y
  {"\001\006\005", "\010\007\004"}, // Z
  {"\377\001", "\377\004"}, // [
  {"\001\004\024\024", "\024\024\001\004"}, // Backslash
  {"\001\377", "\004\377"}, // ]
  {"\010\002", "\024\024"}, // ^
  {"\024\024\024", "\004\004\004"}, // _
};

int writeBigChar(char ch, int x, int y) {
  const char *(*blocks)[2] = NULL; // Pointer to an array of two strings (character pointers)

  if (ch < ' ' || ch > '_') // If outside our table range, do nothing
    return 0;

  blocks = &bigChars[ch - ' ']; // Look up the definition

  for (int half = 0; half <= 1; half++) {
    int t = x; // Write out top or bottom string, byte at a time
    for (const char *cp = (*blocks)[half]; *cp; cp++) {
      lcd.setCursor(t, y + half);
      lcd.write(*cp);
      t = (t + 1) % 40; // Circular scroll buffer of 40 characters, loop back at 40
    }
    lcd.setCursor(t, y + half);
    lcd.write(' '); // Make space between letters, in case overwriting
  }
  return strlen((*blocks)[0]); // Return char width
}

void writeBigString(char *str, int x, int y) {
  char c;
  while ((c = *str++))
    x += writeBigChar(c, x, y) + 1;
}

void setup()
{
  lcd.begin();
  for (int i = 0; i < 8; i++)
    lcd.createChar(i + 1, custom[i]);

  lcd.clear();
  writeBigString("TEST", 0, 0);
  delay(2000);

  lcd.clear();
}

int x = 16; // Start writing the character just off the display, and scroll it in

void loop()
{
  char ch;

  for (ch = ' '; ch <= '_'; ch++) {
    int w = writeBigChar(ch, x, 0); // Write big character just off the scren
    for (int j = 0; j < w + 1; j++) { // Scroll it in
      lcd.scrollDisplayLeft();
      delay(400);
    }
    x = (x + w + 1) % 40; // Adjust our new X, handling circular buffer wrap
  }
}

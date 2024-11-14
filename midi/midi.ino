// Definiciones de la matriz
#define NUM_ROWS 12
#define NUM_COLS 12

// Pines de entrada para las filas
const int rowPins[NUM_ROWS] = {31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53};

// Pines de entrada para las columnas
const int dataPins[NUM_COLS] = {A11, A12, A13, A14, A15, 15, 16, 17, 18, 19, 20, 21};

// Matriz para almacenar el estado actual de las teclas
bool keyState[NUM_ROWS][NUM_COLS];
bool lastKeyState[NUM_ROWS][NUM_COLS];

// Tiempo para el debounce
unsigned long debounceDelay = 50; // milisegundos
unsigned long lastDebounceTime[NUM_ROWS][NUM_COLS];

// Mapa de teclas (notas musicales)
const char* keyMap[NUM_ROWS][NUM_COLS] = {
  {"A5", "A5", "G#5", "G#5", "G5", "G5", "C6", "C6", "B5", "B5", "A#5", "A#5"},
  {"D#5", "D#5", "D5", "D5", "C#5", "C#5", "F#5", "F#5", "F5", "F5", "E5", "E5"},
  {"A4", "A4", "G#4", "G#4", "G4", "G4", "C5", "C5", "B4", "B4", "A#4", "A#4"},
  {"D#4", "D#4", "D4", "D4", "C#4", "C#4", "F#4", "F#4", "F4", "F4", "E4", "E4"},
  {"0", "0", "0", "0", "0", "0", "C4", "C4", "0", "0", "0", "0"},
  {"A3", "A3", "G#3", "G#3", "G3", "G3", "0", "0", "B3", "B3", "A#3", "A#3"},
  {"D#3", "D#3", "D3", "D3", "C#3", "C#3", "F#3", "F#3", "F3", "F3", "E3", "E3"},
  {"A2", "A2", "G#2", "G#2", "G2", "G2", "C3", "C3", "B2", "B2", "A#2", "A#2"},
  {"D#2", "D#2", "D2", "D2", "C#2", "C#2", "F#2", "F#2", "F2", "F2", "E2", "E2"},
  {"A1", "A1", "G#1", "G#1", "G1", "G1", "C2", "C2", "B1", "B1", "A#1", "A#1"},
  {"D#1", "D#1", "D1", "D1", "C#1", "C#1", "F#1", "F#1", "F1", "F1", "E1", "E1"},
  {"0", "0", "0", "0", "0", "0", "C1", "C1", "0", "0", "0", "0"}
};

// Función para convertir notas como "C4" a números MIDI
int noteToMidiNumber(const char* note) {
  if (strcmp(note, "0") == 0) return -1; // Ignorar si es "0"
  
  char noteLetter = note[0]; // Ej. 'C'
  int octave = note[1] - '0';
  if (note[2] != '\0') {
    octave = note[2] - '0';
  }

  int noteNumber;
  switch (noteLetter) {
    case 'C': noteNumber = 0; break;
    case 'D': noteNumber = 2; break;
    case 'E': noteNumber = 4; break;
    case 'F': noteNumber = 5; break;
    case 'G': noteNumber = 7; break;
    case 'A': noteNumber = 9; break;
    case 'B': noteNumber = 11; break;
    default: return -1; // Nota inválida
  }
  
  // Aumenta medio tono para sostenidos
  if (note[1] == '#') noteNumber++;
  
  return noteNumber + (octave + 1) * 12;
}

void sendMidiNoteOn(int midiNote) {
  Serial.write(0x90); // Mensaje de "Note On" en canal 1
  Serial.write(midiNote);
  Serial.write(127);  // Velocidad máxima
}

void sendMidiNoteOff(int midiNote) {
  Serial.write(0x80); // Mensaje de "Note Off" en canal 1
  Serial.write(midiNote);
  Serial.write(0);    // Velocidad 0
}

void setup() {
  Serial.begin(38400);
  
  for (int i = 0; i < NUM_ROWS; i++) {
    pinMode(rowPins[i], INPUT_PULLUP);
  }
  
  for (int i = 0; i < NUM_COLS; i++) {
    pinMode(dataPins[i], OUTPUT);
    digitalWrite(dataPins[i], HIGH);
  }
  
  for (int r = 0; r < NUM_ROWS; r++) {
    for (int c = 0; c < NUM_COLS; c++) {
      keyState[r][c] = false;
      lastKeyState[r][c] = false;
      lastDebounceTime[r][c] = 0;
    }
  }
}

void loop() {
  for (int col = 0; col < NUM_COLS; col++) {
    for (int i = 0; i < NUM_COLS; i++) {
      digitalWrite(dataPins[i], HIGH);
    }

    digitalWrite(dataPins[col], LOW);
    
    for (int row = 0; row < NUM_ROWS; row++) {
      bool reading = digitalRead(rowPins[row]) == LOW;
      
      if (reading != lastKeyState[row][col]) {
        lastDebounceTime[row][col] = millis();
      }
      
      if ((millis() - lastDebounceTime[row][col]) > debounceDelay) {
        if (reading != keyState[row][col]) {
          keyState[row][col] = reading;
          
          int midiNote = noteToMidiNumber(keyMap[col][row]);
          if (midiNote != -1) {
            if (keyState[row][col]) {
              sendMidiNoteOn(midiNote);
            } else {
              sendMidiNoteOff(midiNote);
            }
          }
        }
      }
      
      lastKeyState[row][col] = reading;
    }
    
    digitalWrite(dataPins[col], HIGH);
  }
}

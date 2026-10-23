# STM32 LTC6811

Eine in C implementierte Bibliothek für den Batteriemonitor LTC6811-1 auf STM32-Mikrocontrollern.
Die Ansteuerung erfolgt über SPI und einen isoSPI-Wandler, mehrere LTC6811 lassen sich im Daisy-Chain betreiben.
Über den I2C-Master des LTC6811 werden zusätzlich LTC1380-Multiplexer für Temperatursensoren geschaltet.

1. [Beschreibung](#beschreibung)
2. [Funktionsumfang](#funktionsumfang)
3. [Dateien](#dateien)
4. [Hardware-Konfiguration](#hardware-konfiguration)
5. [Konfigurationsparameter](#konfigurationsparameter)
6. [Datenformat](#datenformat)
7. [Fehlerbilder](#fehlerbilder)
8. [API](#api)
9. [Verwendung](#verwendung)
10. [Hinweise](#hinweise)
11. [Quellen](#quellen)
12. [Abhängigkeiten](#abhängigkeiten)
13. [Lizenz](#lizenz)

## Beschreibung

Der LTC6811 misst bis zu 12 Zellspannungen, 5 GPIO-Spannungen und interne Größen wie Chiptemperatur und Versorgungsspannungen. Jeder Befehl besteht aus einem 11-Bit-Kommando mit 15-Bit-CRC (PEC), jede Registergruppe aus 6 Datenbytes mit eigener PEC. Im Daisy-Chain wird ein Befehl an alle LTC6811 gleichzeitig gesendet (Broadcast), die Daten aller Bausteine laufen hintereinander durch die Chain.

Die Bibliothek ist in drei Schichten aufgeteilt:

- `isospi.c` steuert Chip-Select und SPI und weckt die isoSPI-Strecke, bevor ein Befehl gesendet wird.
- `ltc6811.c` baut Befehle mit PEC, prüft empfangene Daten und verfolgt mit einer Statemaschine, ob der LTC6811 schläft, bereit ist oder gerade misst.
- `ltc1380.c` schreibt I2C-Telegramme in das Communication-Register des LTC6811 und lässt sie an die LTC1380 senden.

Wartezeiten wie Aufwachzeit, Referenzanlauf und Messdauer berechnet die Bibliothek selbst. Lesefunktionen warten, bis eine laufende Messung abgeschlossen ist.

Wie die Module zusammenspielen und welche Zustände es gibt, beschreibt [ARCHITECTURE.md](ARCHITECTURE.md).

## Funktionsumfang

- Befehle, Schreib- und Lesezugriffe auf alle Registergruppen des LTC6811
- PEC-Berechnung (CRC-15) über eine Lookup-Tabelle und PEC-Prüfung je Baustein
- Konfiguration mit Grenzwerten für Unter- und Überspannung, Rücklesen und Vergleich
- Diagnose: ADC-Selbsttests 1 und 2, Thermal Shutdown, Multiplexer-Test, Open-Wire-Erkennung
- Prüfung der Balancing-MOSFETs über die S-Ausgänge
- Kanalwahl an bis zu vier LTC1380 je LTC6811
- Debug-Ausgaben je Modul über `DEBUG_ISOSPI`, `DEBUG_LTC6811` und `DEBUG_LTC1380`

## Dateien

| Datei       | Beschreibung                                                                    |
|-------------|---------------------------------------------------------------------------------|
| `ltc6811.h` | Version, Grenzwerte, Kommandos, Konfigurations- und Balancing-Typen, API        |
| `ltc6811.c` | Statemaschine, Befehle mit PEC, Initialisierung, Diagnose, Balancing            |
| `isospi.h`  | Makros für Enable und Chip-Select, Statemaschine der isoSPI-Strecke, API        |
| `isospi.c`  | Wecken der isoSPI-Strecke, Senden und Empfangen über SPI                        |
| `ltc1380.h` | Adressen und Kanäle der LTC1380, API                                            |
| `ltc1380.c` | I2C-Telegramme für die LTC1380 über das Communication-Register des LTC6811      |

## Hardware-Konfiguration

Einstellungen in STM32CubeMX:

| Einstellung             | Wert                                                                        |
|-------------------------|-----------------------------------------------------------------------------|
| Connectivity → SPI4     | Full-Duplex Master, 8 Bit, MSB first, CPOL High, CPHA 2 Edge (SPI-Mode 3)   |
| SPI4 → Baudrate         | höchstens 1 MHz                                                             |
| SPI4 → NSS              | Software, Chip-Select läuft über einen eigenen GPIO                         |
| GPIO-Ausgang `ISOSPI_CS` | Chip-Select des isoSPI-Wandlers, Ruhepegel High                            |
| GPIO-Ausgang `ISOSPI_EN` | Enable des isoSPI-Wandlers (z. B. LTC6820)                                 |

Die Pin-Labels müssen genau so heißen, weil `isospi.h` die von CubeMX erzeugten Makros `ISOSPI_CS_Pin`, `ISOSPI_CS_GPIO_Port`, `ISOSPI_EN_Pin` und `ISOSPI_EN_GPIO_Port` verwendet. Das SPI-Handle `hspi4` ist in `isospi.c` und `ltc6811.c` fest eingetragen.

## Konfigurationsparameter

| Parameter          | Ort         | Standard | Beschreibung                                                        |
|--------------------|-------------|----------|---------------------------------------------------------------------|
| `LTC6811_DEVICES`  | `ltc6811.h` | 1        | Anzahl der LTC6811 im Daisy-Chain                                   |
| `LTC6811_CELLS`    | `ltc6811.h` | 12       | Zellen je LTC6811                                                   |
| `LTC6811_UVOLT`    | `ltc6811.h` | 625      | Unterspannung als `VUV`, Spannung = (VUV + 1) · 16 · 100 µV         |
| `LTC6811_OVOLT`    | `ltc6811.h` | 2625     | Überspannung als `VOV`, Spannung = VOV · 16 · 100 µV (4,2 V)        |
| `LTC6811_TIMEOUT`  | `ltc6811.h` | 1800     | Watchdog-Zeit in ms, danach gilt der LTC6811 als schlafend          |
| `OPENWIRE_THRESHOLD` | `ltc6811.h` | 4000   | Schwelle für Open-Wire und MOSFET-Prüfung in 100 µV                 |
| `LTC1380_DEVICES`  | `ltc1380.h` | 2        | Anzahl der LTC1380 je LTC6811                                       |

## Datenformat

Schreibfunktionen erwarten 6 Byte je LTC6811, also ein Array mit `6 * LTC6811_DEVICES` Byte. Lesefunktionen liefern 8 Byte je LTC6811 (6 Datenbytes und 2 Byte PEC), das Array braucht `8 * LTC6811_DEVICES` Byte.

Messwerte stehen als 16-Bit-Wert Little Endian in den Registern, ein Bit entspricht 100 µV:

```c
uint16_t zelle1 = (data[1] << 8) | data[0];								// Zelle 1 des ersten LTC6811 in 100 uV
```

`ltc6811_check()` liefert einen Fehlercode, jedes gesetzte Bit steht für einen fehlgeschlagenen Test:

| Bit | Test                     |
|-----|--------------------------|
| 0   | Thermal Shutdown         |
| 1   | Selbsttest 1             |
| 2   | Selbsttest 2             |
| 3   | Multiplexer-Diagnose     |
| 4   | Open-Wire                |

## Fehlerbilder

| Symptom | Ursache | Abhilfe |
|---------|---------|---------|
| `hspi4` nicht deklariert | SPI4 in CubeMX nicht aktiviert | SPI4 nach [Hardware-Konfiguration](#hardware-konfiguration) einrichten |
| `ISOSPI_CS_Pin` nicht deklariert | Pin-Label fehlt oder heißt anders | Pins in CubeMX mit `ISOSPI_CS` und `ISOSPI_EN` benennen |
| `error.h` oder `my_math.h` nicht gefunden | Header gehören nicht zur Bibliothek | im Projekt bereitstellen, siehe [Abhängigkeiten](#abhängigkeiten) |
| Programm bleibt im ersten Befehl hängen | `millis()` zählt nicht hoch, die Statemaschinen warten endlos | Zeitbasis aus STM32_Millis einrichten |
| `ltc6811_read()` liefert immer `false` | isoSPI-Wandler nicht eingeschaltet, falscher SPI-Mode oder `LTC6811_DEVICES` passt nicht zur Chain | `ISOSPI_ENABLE()` vor `ltc6811_init()` aufrufen, SPI-Mode 3 einstellen, Anzahl prüfen |
| Linkerfehler `ITM_SendString` | Debug-Symbol aktiv, SWO-Ausgabe nicht eingebunden | STM32_SerialWireDebug einbinden oder Debug-Symbol auskommentieren |

## API

### LTC6811

```c
bool ltc6811_init (void);                                  // Konfiguration schreiben und zuruecklesen, true = OK
void ltc6811 (uint16_t command);                           // Befehl ohne Daten an alle LTC6811 senden
void ltc6811_write (uint16_t command, uint8_t *data);      // Registergruppe schreiben, 6 Byte je LTC6811
bool ltc6811_read (uint16_t command, uint8_t *data);       // Registergruppe lesen, 8 Byte je LTC6811, true = PEC OK
uint16_t ltc6811_poll (void);                              // Warten bis Wandlung fertig, Rueckgabe Anzahl Abfragen
uint16_t ltc6811_timeout (void);                           // Bitmaske der LTC6811 mit PEC-Fehler beim letzten Lesen
void set_ltc6811_state (LTC6811_State newState);           // Zustand der Statemaschine setzen
LTC6811_State get_ltc6811_state (void);                    // Statemaschine auswerten und Zustand zurueckgeben
void ltc6811_statemaschine (void);                         // Statemaschine auswerten
```

### Diagnose und Balancing

```c
uint8_t ltc6811_check (void);                              // Alle Diagnosen ausfuehren, Rueckgabe Fehlercode
bool ltc6811_test (uint16_t command);                      // ADC-Selbsttest, command = ST1 oder ST2 mit Sampling-Mode
bool ltc6811_thermal (void);                               // Thermal Shutdown pruefen
bool ltc6811_diagn (void);                                 // Multiplexer pruefen
bool ltc6811_openwire (void);                              // Offene Zellleitungen erkennen
bool ltc6811_validate_balance (void);                      // Balancing-MOSFETs pruefen
void ltc6811_balancing (uint8_t cell, bool active);        // Balancing einer Zelle schalten
```

### PEC

```c
uint16_t peccommand (uint16_t command);                    // PEC eines Kommandos
uint16_t peclookup (uint8_t len, uint8_t *data);           // PEC eines Datenarrays
bool peccheck (uint8_t len, uint8_t *data);                // Daten mit PEC pruefen, true = OK
```

### LTC1380

```c
void ltc1380_write (uint8_t Address, uint8_t Channel);     // Kanal eines LTC1380 einschalten
void ltc1380_off (uint8_t Address);                        // Alle Kanaele eines LTC1380 ausschalten
void ltc1380_alloff (void);                                // Alle Kanaele aller LTC1380 ausschalten
```

### Intern

Diese Funktionen nutzt die Bibliothek selbst, im eigenen Programm werden sie nicht benötigt:

```c
void IsoSPI_wakeup (void);                                 // isoSPI-Strecke wecken
void IsoSPI_cmd (uint8_t *command);                        // Kommando senden
void IsoSPI_transmit (uint8_t *command, uint8_t *data);    // Kommando und Daten senden
void IsoSPI_read (uint8_t *command, uint8_t *data);        // Kommando senden und Daten empfangen
void set_IsoSpiState (IsoSpi_State newState);              // Zustand der isoSPI-Statemaschine setzen
IsoSpi_State get_IsoSpiState (void);                       // isoSPI-Statemaschine auswerten und Zustand zurueckgeben
void IsoSPI_statemaschine (void);                          // isoSPI-Statemaschine auswerten
```

## Verwendung

### 1. Dateien einbinden

`ltc6811.*`, `isospi.*` und `ltc1380.*` in das STM32-Projekt kopieren (oder das Repository als Submodul einbinden) und die Header einbinden:

```c
#include "ltc6811.h"
#include "isospi.h"
#include "ltc1380.h"
```

Anzahl der LTC6811 und Grenzwerte in `ltc6811.h` anpassen, siehe [Konfigurationsparameter](#konfigurationsparameter).

### 2. Initialisieren

```c
ISOSPI_ENABLE();														// isoSPI-Wandler einschalten

if (ltc6811_init() != true)
{
	// Konfiguration konnte nicht geschrieben werden
}

if (ltc6811_check() != 0)
{
	// Mindestens eine Diagnose fehlgeschlagen, Bits nach Tabelle Datenformat
}
```

### 3. Zellspannungen messen

`ltc6811_read()` wartet selbst, bis die mit `ltc6811()` gestartete Messung fertig ist:

```c
uint8_t data[8 * LTC6811_DEVICES];										// 8 Byte je LTC6811

ltc6811(ADCVC | MD73 | CELLALL);										// Messung aller Zellen starten

if (ltc6811_read(RDCVA, data) == true)									// Zellen 1 - 3 lesen
{
	uint16_t zelle1 = (data[1] << 8) | data[0];							// in 100 uV
}
```

### 4. Temperatursensor über LTC1380 messen

```c
ltc1380_write(LTC1380_MUX0, TEMPERATUR3);								// Kanal 4 am ersten LTC1380 einschalten
ltc6811(ADAX | MD73 | GPIO1);											// GPIO 1 messen, an dem der Multiplexer haengt
ltc6811_read(RDAUXA, data);												// GPIO 1 - 3 lesen
ltc1380_off(LTC1380_MUX0);												// Kanaele wieder ausschalten
```

An welchem GPIO der Ausgang eines LTC1380 hängt, legt die Platine fest.

## Hinweise

- Alle Funktionen blockieren, bis die Übertragung bzw. Messung abgeschlossen ist. `ltc6811_check()` dauert wegen der Wartezeiten zwischen den Tests mehrere Sekunden und gehört in die Initialisierung, nicht in die Hauptschleife.
- Bekommt der LTC6811 länger als 1,8 s keinen Befehl, schaltet sein Watchdog ab und setzt das Konfigurationsregister zurück. Danach muss `ltc6811_init()` erneut aufgerufen werden.
- Die Zeitvergleiche mit `millis()` stimmen nach dessen Überlauf (etwa 49 Tage Laufzeit) nicht mehr.

### Einschränkungen

- Nur SPI4 (`hspi4`) wird unterstützt.
- Nur der LTC6811-1 mit Broadcast-Befehlen, keine Adressierung einzelner Bausteine (LTC6811-2).
- `ltc6811_balancing()` wertet `cell` noch nicht aus und schaltet immer Zelle 2 des ersten LTC6811.
- `LTC6811_PEC_LOOKUP = 1` kompiliert nicht, die Lookup-Tabelle `pec15Table` ist fest im Code hinterlegt.
- Die Grenzwerte für Temperaturen, Summenspannung sowie Referenz-, Analog- und Digitalversorgung sind definiert, werden aber noch nicht ausgewertet.

## Quellen

- [LTC6811-1 / LTC6811-2 Datenblatt](https://www.analog.com/en/products/ltc6811-1.html)
- [LTC1380 Datenblatt](https://www.analog.com/en/products/ltc1380.html)
- [LTC6820 isoSPI-Wandler](https://www.analog.com/en/products/ltc6820.html)

## Abhängigkeiten

- `main.h` – STM32 HAL, Pin-Labels `ISOSPI_CS` und `ISOSPI_EN`
- `spi.h` – SPI-Handle `hspi4`, von CubeMX erzeugt
- `millis.h` – Zeitbasis der Statemaschinen, [STM32_Millis](https://github.com/Diveturtle93/STM32_Millis)
- `error.h` – `software_error()`, `software_error_debug()` und `ERROR_LTC6811_INITIALTEST`, vom einbindenden Projekt bereitzustellen
- `my_math.h` – `getDifference()`, vom einbindenden Projekt bereitzustellen
- `ITM_SendString()`, `ITM_SendNumber()`, `ITM_SendChar()` – nur mit Debug-Symbolen, [STM32_SerialWireDebug](https://github.com/Diveturtle93/STM32_SerialWireDebug)
- `uartTransmit()`, `uartTransmitNumber()` – nur mit `DEBUG_LTC6811_PEC` oder `DEBUG_LTC6811_VALID_BALANCING`, [STM32_Basicuart](https://github.com/Diveturtle93/STM32_Basicuart)

## Lizenz

Dieses Projekt steht unter der [GPL-3.0 Lizenz](LICENSE).

# STM32 LTC6811
 
Eine in C implementierte Treiberbibliothek für STM32-Mikrocontroller zur Ansteuerung und
Auswertung des Batterie-Zellüberwachungs-ICs **LTC6811-1** von Analog Devices über
**isoSPI**. Zusätzlich wird der analoge Multiplexer **LTC1380** angesteuert, der über den
LTC6811 ausgelesen wird und zur Temperaturmessung mehrerer externer Sensoren dient.
 
## Beschreibung
 
Der LTC6811 überwacht bis zu 12 in Reihe geschaltete Batteriezellen und kommuniziert über
eine galvanisch getrennte isoSPI-Schnittstelle mit dem Mikrocontroller. Diese Bibliothek
implementiert sowohl die isoSPI-Übertragungsschicht als auch die vollständige
Kommandoschnittstelle des LTC6811 gemäß Datenblatt, inklusive PEC-Prüfsummenberechnung
(CRC-15), Selbsttests, Open-Wire-Erkennung und Zell-Balancing.
 
Beide Hauptkomponenten – isoSPI und LTC6811 – werden über eigene Zustandsmaschinen
gesteuert, die zyklisch aufgerufen werden und so eine nicht-blockierende Verarbeitung
ermöglichen.
 
Zusätzlich steuert die Bibliothek den 8-Kanal-Analogmultiplexer **LTC1380**, der über die
GPIO-Pins des LTC6811 angesprochen wird. Damit lassen sich pro LTC6811 bis zu zwei
LTC1380-ICs mit jeweils 8 Temperatursensoren auswerten.
 
## Dateien
 
| Datei         | Beschreibung                                                                 |
|---------------|--------------------------------------------------------------------------------|
| `LTC6811.h`   | Kommando-Codes, Konfigurations-/Balancing-Strukturen, Zustandsmaschinen, API |
| `LTC6811.c`   | Implementierung der LTC6811-Kommunikation, PEC-Berechnung, Diagnosefunktionen |
| `IsoSPI.c`    | Implementierung der isoSPI-Übertragungsschicht und deren Zustandsmaschine    |
| `LTC1380.h`   | Adressen, Kanaldefinitionen und API des Multiplexer-ICs                      |
| `LTC1380.c`   | Implementierung der Multiplexer-Ansteuerung über den LTC6811                |
 
## Allgemeine Konfiguration
 
Die folgenden Konstanten in `LTC6811.h` definieren die Systemtopologie und alle
Grenzwerte für die Diagnose:
 
| Konstante           | Wert   | Beschreibung                                              |
|----------------------|:------:|-------------------------------------------------------------|
| `LTC6811_DEVICES`   | 1      | Anzahl der LTC6811-ICs in der Daisy-Chain                  |
| `LTC6811_CELLS`     | 12     | Anzahl der Zellen pro IC                                   |
| `LTC6811_UVOLT`     | 625    | Unterspannungsschwelle pro Zelle (entspricht 2,0 V)        |
| `LTC6811_OVOLT`     | 2625   | Überspannungsschwelle pro Zelle (entspricht 4,2 V)          |
| `LTC6811_SOC`       | 25200  | Schwelle für die Summenspannung aller Zellen (50,4 V)       |
| `LTC6811_UTEMP`     | 45200  | Untertemperaturschwelle einer Zelle (0 °C)                  |
| `LTC6811_OTEMP`     | 13070  | Übertemperaturschwelle einer Zelle (65 °C)                  |
| `LTC6811_PCB_UTEMP` | 33000  | Untertemperaturschwelle der Platine (0 °C)                  |
| `LTC6811_PCB_OTEMP` | 5760   | Übertemperaturschwelle der Platine (65 °C)                  |
| `LTC6811_DEVUTEMP`  | 20475  | Untertemperaturschwelle des Chips selbst (0 °C)              |
| `LTC6811_DEVOTEMP`  | 25350  | Übertemperaturschwelle des Chips selbst (65 °C)              |
| `OPENWIRE_THRESHOLD`| 4000   | Schwellwert zur Erkennung einer offenen Leitung              |
| `LTC6811_TIMEOUT`   | 1800   | Watchdog-Timeout in ms (1,8 s)                              |
| `SEC_UVOLT` / `SEC_OVOLT` | 29850 / 30150 | Gültigkeitsbereich der zweiten Referenzspannung |
| `ANALOG_UVOLT` / `ANALOG_OVOLT` | 45000 / 59000 | Gültigkeitsbereich der analogen Versorgungsspannung |
| `DIGITAL_UVOLT` / `DIGITAL_OVOLT` | 27000 / 36000 | Gültigkeitsbereich der digitalen Versorgungsspannung |
 
## Zustandsmaschinen
 
### isoSPI-Zustände (`IsoSpi_State`)
 
| Zustand        | Beschreibung                                  |
|----------------|------------------------------------------------|
| `IsoIdle`      | Kommunikation unterbrochen                     |
| `IsoReady`     | Kommunikation kann durchgeführt werden         |
| `IsoActive`    | Kommunikation wird aktiv durchgeführt          |
| `IsoGetReady`  | Kommunikation wird vorbereitet                 |
| `IsoSleep`     | Kommunikation abgeschaltet                     |
 
### LTC6811-Zustände (`LTC6811_State`)
 
| Zustand                | Beschreibung                                                          |
|--------------------------|------------------------------------------------------------------------|
| `LTCStandby`            | IC im Standby, Referenzspannung inaktiv, beide Timer laufen            |
| `LTCMeasure`            | Messung am ADC wird durchgeführt                                      |
| `LTCRefup`              | Referenzspannung aktiv                                                 |
| `LTCSetRefup`           | Referenzspannung wird vorbereitet                                     |
| `LTCWakeup`             | IC wird geweckt                                                         |
| `LTCExtendedBalancing`  | Balancing aktiv, Watchdog-Timer ausgeschaltet, Entladetimer läuft     |
| `LTCSleep`              | IC im Sleep, keine Aktion, beide Timer ausgeschaltet                   |
 
## Datenstrukturen
 
### Konfigurationsregister (`ltc6811_configuration_tag`)
 
Gemeinsame Konfiguration für alle ICs in der Daisy-Chain:
 
| Feld       | Bits | Beschreibung                          |
|------------|:----:|-----------------------------------------|
| `ADCOPT`   | 1    | ADC-Modus-Option                        |
| `DTEN`     | 1    | DTEN-Pin (Discharge Timer Enable)       |
| `REFON`    | 1    | Referenzspannung dauerhaft aktiv         |
| `LTC_GPIO1`–`LTC_GPIO5` | je 1 | Zustand der fünf GPIO-Pins      |
| `VUV`      | 12   | Unterspannungsschwelle                  |
| `VOV`      | 12   | Überspannungsschwelle                   |
 
### Balancing-Register (`ltc6811_balancing_tag`)
 
Individuelle Balancing-Konfiguration je IC:
 
| Feld          | Bits | Beschreibung                          |
|---------------|:----:|-----------------------------------------|
| `DCC1`–`DCC12`| je 1 | Balancing-Aktivierung je Zelle 1–12     |
| `DCTO`        | 4    | Timeout für das Balancing                |
 
## API
 
### IsoSPI-Funktionen
 
```c
void set_IsoSpiState (IsoSpi_State newState);                   // Neuen isoSPI-Zustand setzen
void IsoSPI_statemaschine (void);                                // isoSPI-Zustandsmaschine zyklisch aufrufen
IsoSpi_State get_IsoSpiState (void);                             // Aktuellen isoSPI-Zustand abfragen
void IsoSPI_wakeup (void);                                       // isoSPI aufwecken
void IsoSPI_cmd (uint8_t* command);                              // Kommando über isoSPI senden
void IsoSPI_transmit (uint8_t* command, uint8_t* data);          // Daten über isoSPI senden
void IsoSPI_read (uint8_t* command, uint8_t* data);              // Daten über isoSPI lesen
```
 
### LTC6811-Funktionen
 
```c
void set_ltc6811_state (LTC6811_State newState);                 // Neuen LTC6811-Zustand setzen
void ltc6811_statemaschine (void);                                // LTC6811-Zustandsmaschine zyklisch aufrufen
LTC6811_State get_ltc6811_state (void);                           // Aktuellen LTC6811-Zustand abfragen
void ltc6811 (uint16_t command);                                 // Allgemeines LTC6811-Kommando senden
void ltc6811_write (uint16_t command, uint8_t *data);             // Schreibfunktion
bool ltc6811_read (uint16_t command, uint8_t *data);              // Lesefunktion
uint16_t peccommand (uint16_t command);                          // PEC (CRC-15) für Kommando berechnen
uint16_t peclookup (uint8_t len, uint8_t *data);                 // PEC (CRC-15) für Datenarray berechnen
bool peccheck (uint8_t len, uint8_t *data);                      // PEC validieren
bool ltc6811_init (void);                                        // LTC6811 initialisieren und konfigurieren
uint8_t ltc6811_check (void);                                    // Vollständige Diagnose durchführen
bool ltc6811_test (uint16_t command);                            // Selbsttest 1 oder 2 durchführen
bool ltc6811_thermal (void);                                     // Thermal-Shutdown-Diagnose
bool ltc6811_diagn (void);                                       // Multiplexer-Diagnose
bool ltc6811_openwire (void);                                    // Open-Wire-Leitungscheck
uint16_t ltc6811_poll (void);                                    // ADC-Conversion-Status abfragen
uint16_t ltc6811_timeout (void);                                 // Modul mit Timeout ermitteln
bool ltc6811_validate_balance (void);                             // Balancing-MOSFETs validieren
void ltc6811_balancing (uint8_t cell, bool active);                // Balancing einer Zelle aktivieren/deaktivieren
```
 
### LTC1380-Funktionen (Multiplexer)
 
```c
void ltc1380_write (uint8_t Address, uint8_t Channel);           // Kanal eines ICs auswählen
void ltc1380_off (uint8_t Address);                                // Alle Kanäle eines ICs ausschalten
void ltc1380_alloff (void);                                       // Alle Kanäle aller ICs ausschalten
```
 
## LTC1380-Adressierung
 
| Konstante       | Adresse      | Beschreibung |
|-----------------|:------------:|:--------------|
| `LTC1380_MUX0`  | `0b10010000` | Adresse für IC 1 |
| `LTC1380_MUX1`  | `0b10010010` | Adresse für IC 2 |
| `LTC1380_MUX2`  | `0b10010100` | Adresse für IC 3 |
| `LTC1380_MUX3`  | `0b10010110` | Adresse für IC 4 |
 
| Konstante    | Kanal |
|--------------|:-----:|
| `TEMPERATUR0`–`TEMPERATUR7` | 0–7 (Kanal 1–8 am IC) |
 
Pro LTC6811 sind standardmäßig **zwei** LTC1380-ICs mit je **acht** Sensoren vorgesehen
(`LTC1380_DEVICES`, `LTC1380_SENSORES`).
 
## Befehlssatz (Auszug)
 
Die Bibliothek implementiert den vollständigen Kommandosatz des LTC6811 gemäß Datenblatt
(Tabelle 34), unter anderem:
 
| Kategorie               | Beispielkommandos                                   |
|--------------------------|-------------------------------------------------------|
| Konfiguration            | `WRCFG`, `RDCFG`                                      |
| Zellspannungen           | `RDCVA`–`RDCVD` (Zellen 1–12)                         |
| GPIO / Auxiliary         | `RDAUXA`, `RDAUXB`, `ADAX`, `ADAXD`                   |
| Status                   | `RDSTATA`, `RDSTATB`, `ADSTAT`, `ADSTATD`              |
| ADC-Start                | `ADCVC`, `ADCVAX`, `ADCVSC`, `ADOW`, `ADOL`           |
| Selbsttest               | `CVST`, `AUXST`, `STATST`                              |
| S-Control / Balancing    | `WRSCTRL`, `RDSCTRL`, `WRPWM`, `STSCTRL`               |
| Register löschen         | `CLRCELL`, `CLRAUX`, `CLRSTAT`, `CLRSCTRL`            |
| Diagnose                 | `PLADC`, `DIAGN`                                       |
| Kommunikation (I²C/SPI)  | `WRCOMM`, `RDCOMM`, `STCOMM`                            |
 
Die Bibliothek unterstützt zusätzlich erweiterte Register, die nur bei den Pin-kompatiblen
Varianten **LTC6812** und **LTC6813** verfügbar sind (`WRCFGA/B`, `RDCFGA/B`, `RDCVE/F`,
`RDAUXC/D`).
 
## Verwendung
 
### 1. Dateien einbinden
 
Alle fünf Dateien in das STM32-Projekt kopieren und die Header einbinden:
 
```c
#include "LTC6811.h"
#include "LTC1380.h"
```
 
### 2. Systemtopologie anpassen
 
`LTC6811_DEVICES` und `LTC6811_CELLS` in `LTC6811.h` auf die tatsächliche Anzahl der ICs
und Zellen anpassen. Die Grenzwerte für Über-/Unterspannung und -temperatur können
ebenfalls projektspezifisch angepasst werden.
 
### 3. Initialisierung
 
```c
ltc6811_init();
```
 
### 4. Zustandsmaschinen zyklisch aufrufen
 
```c
while (1)
{
    IsoSPI_statemaschine();
    ltc6811_statemaschine();
}
```
 
### 5. Zellspannungen auslesen und Diagnose durchführen
 
```c
uint8_t fehlerstatus = ltc6811_check();   // führt Selbsttests, Thermal- und Open-Wire-Check durch
```
 
### 6. Balancing steuern
 
```c
ltc6811_balancing(3, true);   // Balancing für Zelle 3 aktivieren
```
 
### 7. Temperatursensoren über LTC1380 auswählen
 
```c
ltc1380_write(LTC1380_MUX0, TEMPERATUR2);   // Kanal 3 am ersten Multiplexer auswählen
```
 
## Hinweise
 
- Alle PEC-Prüfsummen (CRC-15) werden über eine vorab berechnete Lookup-Tabelle (`peclookup`)
  validiert, um die Integrität jeder isoSPI-Übertragung sicherzustellen.
- Debug-Ausgaben für isoSPI, LTC6811 allgemein, PEC-Berechnung und Balancing-Validierung
  können einzeln über die Makros `DEBUG_ISOSPI`, `DEBUG_LTC6811`, `DEBUG_LTC6811_PEC` und
  `DEBUG_LTC6811_VALID_BALANCING` in `LTC6811.h` aktiviert werden (nur im Debug-Build wirksam).
- Diese Bibliothek wird im [EAuto_BMS](https://github.com/Diveturtle93/EAuto_BMS) zur
  Zellüberwachung des Hochvolt-Akkumulators eingesetzt.
  
## Abhängigkeiten
 
- `main.h` – STM32 HAL (inkl. SPI-Handle für isoSPI)

## Lizenz
 
Dieses Projekt steht unter der [GPL-3.0 Lizenz](LICENSE).

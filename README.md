Klar! Hier ist eine kürzere und prägnante Version für die GitHub-Repository-Beschreibung:

---

# Binäre Uhr – ATmega48A

Dieses Repository enthält den Quellcode für eine binäre Uhr, die mit einem **ATmega48A** Mikrocontroller programmiert wurde. Eine detaillierte Erklärung zur Funktionsweise und Implementierung findest du in der beigefügten PDF-Datei:  
👉 **[Binäre Uhr.pdf](./Bineare_Uhr.pdf)**
👉 **[Schaltplan_KiCad.pdf](./Schaltplan_KiCad.pdf)**

## 📌 Projektbeschreibung

Die binäre Uhr zeigt die Uhrzeit in binärer Form mit LEDs an. Sie bietet:

- **Helligkeitseinstellung** (PWM)
- **Speicherung des letzten Zustands** im EEPROM
- **Drei Energiesparmodi**:  
  - Zwei **Idle-Modi** (geringer Stromverbrauch, Timer bleibt aktiv)
  - **Power-Down-Modus** (maximale Energieeinsparung, Weckung per Tasteninterrupt)

Das Programm wird mit **GccApplication1** entwickelt und mit **AVRDude** auf den Mikrocontroller geflasht.

## 🔧 Anforderungen

- **Mikrocontroller:** ATmega48A
- **AVR-GCC Toolchain**  
- **AVRDude** für das Flashen

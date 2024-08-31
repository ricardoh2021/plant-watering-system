# Project Description and Credits

**Original Author:** Leo Marte, Computer Science, Gettysburg College class of 2009.

## Project Description

This project combines concepts from two distinct projects: **Botanicalls** and **Growduino**. The primary objective is to integrate the knowledge from both sources to create an autonomous plant watering system. The system detects moisture levels in the plant and powers a water pump when the plant requires water, effectively allowing the plant to be watered without human intervention, assuming the reservoir is refilled regularly.

### Challenges and Calibration

One of the significant challenges was translating electrical readings from the plant into a meaningful state (dry, moist, or soaked). Extensive experimentation was conducted with two plants, Ralph and Lucy, to determine appropriate moisture readings:

- For Lucy, after being left without water for five days, the sensor reading was approximately 420 units, which was deemed the **DRY** state.
- Adding a full cup of water to Lucy resulted in a reading of about 1020 units, which was set as the **SOAKED** state.
- The midpoint between **DRY** and **SOAKED** was chosen as the **MOIST** state, calculated to be 710 units.

The Botanicalls project provided a useful tutorial on DIY sensors. Although the initial code and circuit were created without a relay, the wiring was prepared for future integration of a relay to complete the plant-watering system.

### Fault Tolerance

The project includes fault tolerance to avoid overwatering and flooding. If the moisture levels do not change after a watering action, the system will suspend operation, and the power light will blink to indicate a need for servicing.

### Controller Box Description

The controller box has two cables:

- **USB Cable:** Connects to a computer running Arduino software or a serial terminal for data communication.
- **Wall Plug:** Provides power to the water pump.

Inside the box:

- The extension cable is split to connect to the relay and the Arduino.
- The remaining extension cord with three outlets extends out of the box and connects to the plant probes.

The outlets are only live when the Arduino signals the relay to close and activate the water pump. The relay stays closed for approximately three seconds by default, ensuring the plant receives water without human intervention.

### Troubleshooting

This file is particularly useful for isolating just the watering system and troubleshooting the moisture sensors independently. It allows you to test and refine the moisture sensing functionality separately from the overall plant-watering system.

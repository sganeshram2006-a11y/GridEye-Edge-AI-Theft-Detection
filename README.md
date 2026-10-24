# GridEye-Edge-AI-Theft-Detection
GridEye is an IoT-based Edge-AI system that monitors power quality and detects electricity theft by comparing supply and consumer readings. Using ESP32, PZEM-004T sensors, and Random Forest ML, it classifies normal, partial theft, direct theft, and idle conditions to help reduce power losses and improve distribution monitoring.


# GridEye: Edge-AI Real-Time Power Quality Monitoring and Differential Theft Detection Using IoT

## 1. Project Overview

GridEye is an Edge-AI and IoT-based electricity monitoring system designed to monitor power quality and identify potential electricity theft. The prototype uses an ESP32, two PZEM-004T v3 energy-monitoring sensors, and a Random Forest machine learning model to classify different operating conditions.

The system processes electrical measurements, classifies configured operating conditions locally on the ESP32, and sends electrical data to ThingSpeak for remote monitoring.

## 2. Problem Statement

Electricity theft and non-technical power losses create financial challenges for electricity distribution companies. Identifying abnormal consumption patterns can help highlight situations that require further investigation.

GridEye explores an affordable approach using embedded systems, machine learning, and IoT to support electrical monitoring and identify potential theft conditions.

## 3. Objectives

- Monitor electrical parameters in real time.
- Analyse power, current, and voltage differences.
- Classify operating conditions using a Random Forest model.
- Execute the converted model on the ESP32.
- Send electrical measurements to ThingSpeak.
- Develop a prototype that can be extended for future distribution-level monitoring.

## 4. Key Features

- **Real-time monitoring:** Measures voltage, current, power, and power factor.
- **Edge-AI classification:** Runs the converted Random Forest model on the ESP32.
- **Four operating conditions:** Normal Operation, Partial Theft, Direct Theft, and Idle.
- **Differential analysis:** Uses calculated power, current, and voltage differences as model inputs.
- **IoT monitoring:** Sends electrical readings to ThingSpeak for visualisation.
- **Expandable design:** Provides a foundation for future improvements in electricity monitoring.

## 5. Technology Stack

| Technology | Purpose |
|---|---|
| ESP32 | Embedded processing and Edge-AI execution |
| PZEM-004T v3 | Electrical parameter measurement |
| Random Forest | Operating-condition classification |
| C/C++ | ESP32 firmware and converted model |
| Python | Model training and evaluation |
| scikit-learn | Machine learning |
| pandas and NumPy | Dataset processing |
| Joblib | Python model serialisation |
| ThingSpeak | Cloud monitoring and visualisation |
| Arduino IDE | Firmware development |

## 6. System Architecture

The prototype follows this workflow:

1. Two PZEM-004T v3 sensors measure electrical parameters in the configured supply-side and consumer-side test arrangements.
2. The ESP32 collects the sensor readings.
3. The firmware calculates the required differential features.
4. The converted Random Forest model processes the 11 input features.
5. The model classifies the operating condition.
6. Electrical measurements and derived values are uploaded to ThingSpeak for monitoring.

### Model Input Features

- `V1`, `I1`, `P1`, `PF1`
- `V2`, `I2`, `P2`, `PF2`
- `Power_Diff`
- `Current_Diff`
- `Voltage_Diff`

The model uses the feature order expected by the deployed implementation.

## 7. Classification Categories

| Category | Description |
|---|---|
| Normal Operation | Consumer-only load condition |
| Partial Theft | Both test loads active under the configured test condition |
| Direct Theft | Configured bypass/direct-theft test condition |
| Idle | Both test loads switched off |

These categories represent conditions configured for the prototype. Real-world theft identification requires further validation under diverse field conditions.

## 8. Hardware Requirements

- ESP32 development board
- Two PZEM-004T v3 energy-monitoring modules
- Suitable electrical test loads, such as the prototype's 9 W bulbs
- Appropriate wiring and electrical protection
- Computer for firmware development

**Safety:** Mains-voltage experiments must be performed with appropriate isolation and protection under qualified supervision. Never handle exposed live wiring.

## 9. Software Requirements

- Arduino IDE
- ESP32 board support package
- Compatible PZEM-004T library
- Converted Random Forest C/C++ model header
- Python 3 for model training and evaluation

Python dependencies:

- NumPy
- pandas
- scikit-learn
- Joblib

## 10. Repository Structure

```text
GridEye/
├── README.md
├── LICENSE
├── .gitignore
├── esp32/
│   ├── GridEye.ino
│   └── gridEye_random_forest_model.h
├── ml/
│   └── retrain_grideye_model.py
├── data/
│   └── GridEye_combined_clean_dataset.csv
├── docs/
│   └── TECHNICAL_DESIGN.md
├── tests/
└── requirements.txt
```

The repository contains the ESP32 firmware, converted Random Forest model, Python training script, and supporting project documentation. The Python `.pkl` model file is not included.

## 11. Installation and Setup

### A. ESP32 Firmware

1. Install Arduino IDE and configure ESP32 board support.
2. Install the required PZEM-004T library.
3. Place `GridEye.ino` and `gridEye_random_forest_model.h` in the appropriate Arduino sketch directory.
4. Ensure the sketch folder, `.ino` filename, and model-header `#include` statement match.
5. Configure the sensor communication pins according to the actual hardware wiring.
6. Configure Wi-Fi and ThingSpeak credentials locally.
7. Select the correct ESP32 board and port.
8. Compile and upload the firmware.
9. Open Serial Monitor to inspect electrical readings and classification outputs.

### B. Python Model Training

The Python training script is provided separately for model retraining and evaluation.

Install the dependencies:

```bash
pip install numpy pandas scikit-learn joblib
```

Place `GridEye_combined_clean_dataset.csv` in the working directory expected by the script, then run:

```bash
python ml/retrain_grideye_model.py
```

The script generates a Python `.pkl` model and a text evaluation report locally. The `.pkl` file is not included in this repository. Retraining the Python model does not automatically update the converted model running on the ESP32.

### C. ThingSpeak Configuration

1. Create a ThingSpeak channel.
2. Configure its fields to match the measurements uploaded by the firmware.
3. Set the required channel credentials in your local configuration.
4. Verify that the readings appear correctly on the dashboard.

**Security:** Never upload Wi-Fi passwords, ThingSpeak write API keys, Firebase credentials, or other private tokens to a public repository.

## 12. Dataset and Model Evaluation

The Python training workflow expects a labelled dataset containing the 11 model features and a target column identifying each operating condition.

The training script uses an 80:20 stratified train-test split and reports:

- Held-out test accuracy
- Precision, recall, and F1-score
- Confusion matrix
- Five-fold cross-validation scores

Evaluation results depend on the dataset quality and experimental conditions. Report the actual measured results alongside the dataset size and testing limitations. No unverified accuracy claim is made here.

## 13. Testing

Recommended test cases include:

- Normal consumer-load operation
- Partial-theft test arrangement
- Direct-theft/bypass test arrangement
- Both test loads switched off
- Invalid or missing sensor readings
- Wi-Fi disconnection and reconnection
- Comparison of predicted classes with expected test labels

Record the expected output, observed output, and pass/fail status for each test performed.

## 14. Expected Outcomes and Impact

GridEye aims to demonstrate how embedded AI and IoT can support real-time electrical monitoring and identify configured abnormal consumption conditions.

Potential benefits include improved visibility into electrical operating conditions, support for investigating possible non-technical losses, and a foundation for future electricity distribution monitoring solutions.

## 15. Limitations

- The current implementation is a controlled prototype, not a validated utility-scale theft detection system.
- Classification performance depends on the training data and operating conditions.
- Sensor inaccuracies and changes in electrical loads may affect predictions.
- Field validation is required before considering deployment in actual distribution networks.
- GIS-based monitoring, feeder-level deployment, and advanced low-connectivity alerts are future extensions unless implemented separately.

## 16. Future Scope

- Expand testing with more load combinations and operating conditions.
- Improve robustness against sensor errors and communication failures.
- Develop a more comprehensive monitoring dashboard.
- Investigate communication technologies for areas with limited internet connectivity.
- Explore feeder-level monitoring and GIS-based visualisation.

## 17. License

This project is shared under the terms specified in the repository's `LICENSE` file. Check third-party library, dataset, and model-conversion tool licences before redistributing their components.

## 18. Acknowledgements

Developed as an Electrical and Electronics Engineering project exploring embedded systems, machine learning, and IoT for smarter electricity monitoring.

---

**Disclaimer:** GridEye is intended for research and educational purposes. Its classifications represent configured prototype test conditions and should not be treated as conclusive proof of electricity theft without independent investigation.

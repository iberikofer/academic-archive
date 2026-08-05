# 📑 Laboratory Work #5

> **Course:** Human-Computer Interaction (HCI)  
> **Institution:** Vinnytsia National Technical University (VNTU)  
> **Language:** English

---

## 🎯 Goal

Develop a voice-controlled AI terminal assistant (J.A.R.V.I.S.) in Python using Tkinter GUI, SpeechRecognition API, Google Gemini AI API, gTTS speech synthesis, and custom voice macro execution.

---

## 💻 Code & Resources

- **Main Script:** [`jarvis.py`](./jarvis.py)
- **Voice Macros:** [`actions.py`](./actions.py)
- **Dependencies List:** [`requirements.txt`](./requirements.txt)
- **Course Guidelines:** [HCI Guidelines (PDF)](../ЛР_Людино-Машинна_Взаємодія_Методичні_вказівки.pdf)

---

## 🚀 How to Run

1. Install required dependencies:

```bash
pip install -r requirements.txt
```

2. Set your Gemini API Key in environment variables:

```bash
export GEMINI_API_KEY="your_api_key_here" # Linux / macOS
$env:GEMINI_API_KEY="your_api_key_here"   # Windows PowerShell
```

3. Launch application:

```bash
python jarvis.py
```
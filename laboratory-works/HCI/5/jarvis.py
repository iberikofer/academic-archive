import tkinter as tk
import speech_recognition as sr
import threading
from actions import VoiceMacros
import pyttsx3
import google.generativeai as genai
from gtts import gTTS
import pygame
import os

# pip install -r requirements.txt
# python jarvis.py

class SpeechToTextApp:
    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("J.A.R.V.I.S. Головний термінал")
        self.root.configure(bg="#050505")
        self.root.attributes("-fullscreen", True)

        self.root.bind(
            "<Escape>", lambda event: self.root.attributes("-fullscreen", False)
        )
        self.recognizer = sr.Recognizer()
        self.is_listening = False
        self.macros = VoiceMacros()
        api_key = os.getenv("GEMINI_API_KEY", "")
        genai.configure(api_key=api_key)  # type: ignore
        self.ai_model = genai.GenerativeModel(  # type: ignore
            "gemini-2.5-flash",
            system_instruction="Ти J.A.R.V.I.S., штучний інтелект та голосовий асистент. Відповідай українською мовою. Твої відповіді мають бути максимально короткими, технічними та лаконічними, оскільки вони будуть озвучуватися синтезатором мовлення. Не використовуй форматування Markdown.",
        )
        self.chat_session = self.ai_model.start_chat(history=[])

        pygame.mixer.init()

        self.setup_ui()

    def setup_ui(self) -> None:
        header_frame = tk.Frame(self.root, bg="#050505")
        header_frame.pack(pady=20)

        self.main_title = tk.Label(
            header_frame,
            text="J.A.R.V.I.S.",
            fg="#00ffcc",
            bg="#050505",
            font=("Consolas", 94, "bold"),
        )
        self.main_title.pack(pady=(0, 10))

        self.status_label = tk.Label(
            header_frame,
            text="СТАТУС: РЕЖИМ ОЧІКУВАННЯ",
            fg="#00ffcc",
            bg="#050505",
            font=(
                "Consolas",
                24,
                "bold",
            ),
        )
        self.status_label.pack()

        btn_frame = tk.Frame(self.root, bg="#050505")
        btn_frame.pack(pady=10)

        self.start_btn = tk.Button(
            btn_frame,
            text="ЗАПУСК",
            command=self.start_listening,
            bg="#111111",
            fg="#00ffcc",
            activebackground="#00ffcc",
            activeforeground="#000000",
            font=("Consolas", 24, "bold"),
            relief=tk.FLAT,
            width=20,
        )
        self.start_btn.pack(side=tk.LEFT, padx=10)

        self.stop_btn = tk.Button(
            btn_frame,
            text="СТОП",
            command=self.stop_listening,
            bg="#111111",
            fg="#ff3333",
            activebackground="#ff3333",
            activeforeground="#000000",
            font=("Consolas", 24, "bold"),
            relief=tk.FLAT,
            state=tk.DISABLED,
            width=20,
        )
        self.stop_btn.pack(side=tk.LEFT, padx=10)

        terminal_frame = tk.Frame(
            self.root, bg="#050505", highlightbackground="#00ffcc", highlightthickness=1
        )
        terminal_frame.pack(pady=20, padx=20, fill=tk.BOTH, expand=True)

        self.text_area = tk.Text(
            terminal_frame,
            wrap=tk.WORD,
            bg="#050505",
            fg="#00ffcc",
            font=("Consolas", 24),
            insertbackground="#00ffcc",
            relief=tk.FLAT,
            padx=10,
            pady=10,
        )
        self.text_area.pack(fill=tk.BOTH, expand=True)
        self.update_text(
            "> Базові модулі завантажено.\n> Система готова до роботи.\n\n"
        )

    def start_listening(self) -> None:
        self.is_listening = True
        self.start_btn.config(state=tk.DISABLED)
        self.stop_btn.config(state=tk.NORMAL)
        self.status_label.config(text="СТАТУС: СЛУХАЮ...", fg="#00ffcc")
        self.update_text("> [J.A.R.V.I.S] Захоплення аудіо ініціалізовано.\n")

        threading.Thread(target=self.listen_loop, daemon=True).start()

    def stop_listening(self) -> None:
        self.is_listening = False
        self.start_btn.config(state=tk.NORMAL)
        self.stop_btn.config(state=tk.DISABLED)
        self.status_label.config(text="РЕЖИМ ОЧІКУВАННЯ", fg="#555555")
        self.update_text("> [J.A.R.V.I.S] Захоплення аудіо припинено.\n")

    def listen_loop(self) -> None:
        with sr.Microphone() as source:
            self.recognizer.adjust_for_ambient_noise(source, duration=0.5)
            while self.is_listening:
                try:
                    audio = self.recognizer.listen(
                        source, timeout=1, phrase_time_limit=10
                    )
                    self.process_audio(audio)
                except sr.WaitTimeoutError:
                    continue
                except Exception as e:
                    if self.is_listening:
                        self.update_text(f"> [ERR] Системна помилка: {e}\n")

    def process_audio(self, audio: sr.AudioData) -> None:
        try:
            text = self.recognizer.recognize_google(audio, language="uk-UA")  # type: ignore
            self.update_text(f"> КОРИСТУВАЧ: {text}\n")

            macro_response = self.macros.execute(text)

            if macro_response:
                self.update_text(f"> ВИКОНАНО: {macro_response}\n\n")
                self.speak(macro_response)
            else:
                self.ask_ai(text)

        except sr.UnknownValueError:
            pass
        except sr.RequestError as e:
            self.update_text(f"> [ERR] Втрачено з'єднання з API: {e}\n")

    def update_text(self, text: str) -> None:
        self.root.after(0, self._insert_text, text)

    def _insert_text(self, text: str) -> None:
        self.text_area.insert(tk.END, text)
        self.text_area.yview(tk.END)

    def speak(self, text: str) -> None:
        def _speak_thread():
            filename = "temp_voice.mp3"
            try:
                tts = gTTS(text=text, lang="uk")
                tts.save(filename)

                pygame.mixer.music.load(filename)
                pygame.mixer.music.play()

                while pygame.mixer.music.get_busy():
                    pygame.time.Clock().tick(10)

                pygame.mixer.music.unload()
                os.remove(filename)

            except Exception as e:
                self.update_text(f"> [ERR] Помилка озвучки: {e}\n")
                if os.path.exists(filename):
                    try:
                        os.remove(filename)
                    except Exception:
                        pass

        threading.Thread(target=_speak_thread, daemon=True).start()

    def ask_ai(self, text: str) -> None:
        self.update_text("> [J.A.R.V.I.S] Обробка запиту через нейромережу...\n")

        def _ai_thread():
            try:
                response = self.chat_session.send_message(text)
                clean_text = response.text.replace("*", "").strip()
                self.update_text(f"> [J.A.R.V.I.S]: {clean_text}\n\n")
                self.speak(clean_text)
            except Exception as e:
                self.update_text(f"> [ERR] Помилка ШІ: {e}\n")
                self.speak("Помилка зв'язку з головним сервером нейромережі.")

        threading.Thread(target=_ai_thread, daemon=True).start()


if __name__ == "__main__":
    root = tk.Tk()
    app = SpeechToTextApp(root)
    root.mainloop()

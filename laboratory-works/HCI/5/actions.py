import os
import subprocess
import webbrowser
import datetime
import ctypes
import platform
import socket
import random
import shutil

try:
    import psutil
except ImportError:
    psutil = None

try:
    import pyperclip
except ImportError:
    pyperclip = None


class VoiceMacros:
    def __init__(self):
        self.jokes = [
            "Скільки програмістів треба, щоб замінити лампочку? Жодного — це апаратна проблема.",
            "Програміст іде в магазин. Дружина: 'Купи молоко, і якщо будуть яйця — візьми десяток'. Він повернувся з 10 пляшками молока.",
            "— Чому ти не спиш? — Завтра дедлайн. — То лягай. — Але я сплю! (sudo sleep)",
            "Алгоритм — це рецепт для людини, яка не вміє готувати, але може читати інструкцію.",
            "Існує 10 типів людей: ті, хто розуміє двійкову систему, і ті, хто ні.",
            "Чому Java-програмісти носять окуляри? Тому що вони не бачать Sharp (C#).",
            "Оптиміст каже, що склянка наполовину повна. Песиміст — що наполовину порожня. Програміст каже, що склянка вдвічі більша, ніж потрібно.",
            "Рядок коду, який ти написав пів року тому і не прокоментував, автоматично стає чужим кодом.",
            "Програміст дивиться на код з багом: 'Як це взагалі працювало?!' Після рефакторингу: 'Як це взагалі працює?!'",
            "Справжній програміст стає релігійним лише тоді, коли падає продакшн у п'ятницю ввечері.",
            "CSS як магія: ти міняєш один padding, і весь твій макет летить у чорну діру.",
            "Два розробники розмовляють: — Я написав штучний інтелект. — І що він робить? — Плаче, коли бачить мій legacy-код.",
            "— Тату, чому сонце щодня встає на сході й сідає на заході? — Синку, воно працює? Нічого не чіпай!",
            "Сеньйор — це джуніор, який точно знає, де саме шукати відповіді на Stack Overflow.",
        ]
        self.joke_idx = 0

        self.facts = [
            "Перший 'баг' — буквальний жук, знайдений у реле Mark II у 1947 році.",
            "Python названо не на честь змії, а на честь комедійного шоу 'Monty Python'.",
            "Git був створений Лінусом Торвальдсом за два тижні у 2005 році.",
            "JavaScript та Java не мають між собою нічого спільного — лише маркетинг.",
            "Перший жорсткий диск IBM (1956) важил 971 кг і зберігав лише 5 МБ.",
            "Перший комп'ютерний вірус (Creeper) з'явився у 1971 році та просто виводив напис 'Я Creeper, спіймай мене'.",
            "Ада Лавлейс вважається першим у світі програмістом — вона написала алгоритм для аналітичної машини Беббіджа у 1843 році.",
            "Слово 'кіберпростір' вперше з'явилося у науково-фантастичному романі Вільяма Гібсона 'Нейромант' (1984).",
            "Близько 90% усіх світових грошей зараз існують виключно в цифровому вигляді на серверах банків.",
            "Формат зображень PNG спочатку розшифровувався як 'Png's Not GIF' — вільна заміна патентованому тоді GIF.",
            "Компанія Google спочатку називалася 'BackRub', оскільки їхній алгоритм аналізував зворотні посилання сайту.",
            "У першій версії комп'ютерної миші (1964) шнур виходив із задньої частини пристрою, що нагадувало хвіст.",
            "Найдовша працююча програма в історії — система Sabre (1964), яка досі містить частини початкового коду.",
            "Linux kernel містить понад 30 мільйонів рядків коду, і більша його частина написана ентузіастами.",
            "Символ @ (собачка) був обраний для емейлів у 1971 році, щоб відокремити ім'я користувача від назви хоста.",
        ]
        self.fact_idx = 0

        self.quotes = [
            "«Будь-яка розвинена технологія невідрізнима від магії.» — Артур Кларк",
            "«Спочатку вирішіть задачу. Потім напишіть код.» — Джон Джонсон",
            "«Програми мають бути написані для людей.» — Абельсон і Сасман",
            "«Найкращий код — це той, якого не треба писати.»",
            "«Не зупиняйся, поки не пишатимешся результатом.»",
            "«Помилятися людині властиво, але для справжнього хаосу потрібен комп'ютер.» — Роберт Орбен",
            "«Простота — запорука надійності.» — Едсгер Дейкстра",
            "«Будь-який дурень може написати код, який зрозуміє комп'ютер. Хороші програмісти пишуть код для людей.» — Мартін Фаулер",
            "«Вимірювати прогрес розробки кількістю рядків коду — це як оцінювати будівництво літака за його вагою.» — Білл Гейтс",
            "«Перші 90% коду займають 90% часу. Решта 10% займають інші 90% часу.» — Том Каргілл",
            "«Хороший розробник дивиться в обидва боки, коли переходить вулицю з одностороннім рухом.» — Дуг Ліндер",
            "«Архітектура — це рішення, які важко змінити пізніше.» — Мартін Фаулер",
            "«Якщо ви вважаєте користувачів ідіотами, то тільки ідіоти будуть користуватися вашим софтом.» — Лінус Торвальдс",
            "«Немає нічого більш постійного, ніж тимчасовий костиль у коді.»",
            "«Код не бреше, коментарі іноді можуть.» — Рон Джефріс",
        ]
        self.quote_idx = 0

        raw_commands = {
            ("відкрий сайт", "перейди на"): self.open_website,
            (
                "гугли",
                "пошук",
                "знайди",
                "google",
                "знайди в інтернеті",
            ): self.search_google,
            ("вікіпедія", "wiki", "знайди у вікіпедії"): self.search_wikipedia,
            ("стаковерфлоу", "stackoverflow"): self.search_stackoverflow,
            ("пошук відео", "знайди відео", "ютуб пошук"): self.search_youtube,
            ("пошук гітхаб", "знайди репозиторій", "пошук репо"): self.search_github,
            ("пошук npm", "знайди пакет", "npm"): self.search_npm,
            ("пошук pypi", "знайди python пакет", "pypi"): self.search_pypi,
            (
                "пошук images",
                "знайди зображення",
                "гугл картинки",
            ): self.search_google_images,
            ("маршрут до", "як дістатись"): self.search_maps,
            ("пошук новини", "останні новини про"): self.search_news,
            ("тлумачення слова", "що означає", "словник"): self.search_dictionary,
            ("перекласти", "переклад слова"): self.translate_word,
            (
                "відкрий гугл",
                "запусти гугл",
                "гугл хром",
                "хром",
                "відкрий браузер",
                "запусти браузер",
            ): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://google.com"), "Відкрито Google"
            ),
            ("перекладач", "переклади", "гугл перекладач"): lambda arg: (
                self._run_and_return(
                    lambda: webbrowser.open("https://translate.google.com"),
                    "Відкрито перекладач",
                )
            ),
            ("діпл", "deepl", "кращий перекладач"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://deepl.com"), "Відкрито DeepL"
            ),
            ("мапи", "карта", "маршрут", "гугл карти"): lambda arg: (
                self._run_and_return(
                    lambda: webbrowser.open("https://maps.google.com"), "Відкрито мапи"
                )
            ),
            ("погода", "прогноз", "яка зараз погода"): lambda arg: self._run_and_return(
                lambda: webbrowser.open(
                    "https://weather.com/weather/today/l/86e099d7159ab6f5069c464964885c26166dd051eb6d987e286c6f02b9a92c58"
                ),
                "Відкрито прогноз погоди",
            ),
            ("новини", "останні новини", "укрнет"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://ukr.net"), "Відкрито новини"
            ),
            ("ютуб", "youtube", "включи ютуб", "відкрий ютуб"): lambda arg: (
                self._run_and_return(
                    lambda: webbrowser.open("https://youtube.com"), "Відкрито YouTube"
                )
            ),
            ("гітхаб", "github"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://github.com"), "Відкрито GitHub"
            ),
            ("чат джіпіті", "chatgpt", "відкрий чат"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://chatgpt.com"), "Відкрито ChatGPT"
            ),
            ("джеміні", "gemini"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://gemini.google.com"), "Відкрито Gemini"
            ),
            ("клод", "claude", "штучний інтелект"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://claude.ai"), "Відкрито Claude"
            ),
            ("твіч", "twitch"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://twitch.tv"), "Відкрито Twitch"
            ),
            ("реддіт", "reddit"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://reddit.com"), "Відкрито Reddit"
            ),
            ("фейсбук", "facebook", "відкрий фейсбук"): lambda arg: (
                self._run_and_return(
                    lambda: webbrowser.open("https://facebook.com"), "Відкрито Facebook"
                )
            ),
            ("інстаграм", "instagram", "інста"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://instagram.com"), "Відкрито Instagram"
            ),
            ("тікток", "tiktok"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://tiktok.com"), "Відкрито TikTok"
            ),
            ("твіттер", "ікс", "twitter"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://x.com"), "Відкрито X"
            ),
            ("пошта", "gmail", "електронна пошта", "гмейл"): lambda arg: (
                self._run_and_return(
                    lambda: webbrowser.open("https://mail.google.com"), "Відкрито пошту"
                )
            ),
            ("нетфлікс", "netflix", "фільми"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://netflix.com"), "Відкрито Netflix"
            ),
            ("фігма", "figma", "дизайн"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://figma.com"), "Відкрито Figma"
            ),
            ("локалхост", "localhost", "локальний сервер"): lambda arg: (
                self._run_and_return(
                    lambda: webbrowser.open("http://localhost"), "Відкрито Localhost"
                )
            ),
            ("аліекспрес", "aliexpress"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://aliexpress.com"), "Відкрито AliExpress"
            ),
            ("амазон", "amazon"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://amazon.com"), "Відкрито Amazon"
            ),
            ("розетка", "rozetka"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://rozetka.com.ua"), "Відкрито Rozetka"
            ),
            ("олх", "olx"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://olx.ua"), "Відкрито OLX"
            ),
            ("лінкедін", "linkedin"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://linkedin.com"), "Відкрито LinkedIn"
            ),
            ("пінтерест", "pinterest"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://pinterest.com"), "Відкрито Pinterest"
            ),
            ("гугл диск", "google drive"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://drive.google.com"),
                "Відкрито Google Drive",
            ),
            ("гугл докс", "документи гугл"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://docs.google.com"),
                "Відкрито Google Docs",
            ),
            ("гугл фото", "google photos"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://photos.google.com"),
                "Відкрито Google Photos",
            ),
            ("medium", "медіум", "статті"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://medium.com"), "Відкрито Medium"
            ),
            ("дев.то", "dev.to"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://dev.to"), "Відкрито Dev.to"
            ),
            ("кодпен", "codepen", "онлайн редактор"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://codepen.io"), "Відкрито CodePen"
            ),
            ("рітсплас", "replit", "онлайн код"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://replit.com"), "Відкрито Replit"
            ),
            ("корутера", "coursera", "курси"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://coursera.org"), "Відкрито Coursera"
            ),
            ("удемі", "udemy", "відеокурси"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://udemy.com"), "Відкрито Udemy"
            ),
            ("докер хаб", "docker hub"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://hub.docker.com"), "Відкрито Docker Hub"
            ),
            ("vercel", "верцел"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://vercel.com"), "Відкрито Vercel"
            ),
            ("нетліфай", "netlify"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://netlify.com"), "Відкрито Netlify"
            ),
            ("хероку", "heroku"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://heroku.com"), "Відкрито Heroku"
            ),
            ("дропбокс", "dropbox"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://dropbox.com"), "Відкрито Dropbox"
            ),
            ("ноушн", "notion", "нотатки"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://notion.so"), "Відкрито Notion"
            ),
            ("трело", "trello", "дошка задач"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://trello.com"), "Відкрито Trello"
            ),
            ("джіра", "jira", "баг трекер"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://jira.atlassian.com"), "Відкрито Jira"
            ),
            ("слак", "slack", "робочий чат"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://slack.com"), "Відкрито Slack"
            ),
            ("зум", "zoom", "відеоконференція"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://zoom.us"), "Відкрито Zoom"
            ),
            ("міт", "meet", "гугл міт"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://meet.google.com"),
                "Відкрито Google Meet",
            ),
            ("антропік", "anthropic"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://anthropic.com"), "Відкрито Anthropic"
            ),
            ("хагінг фейс", "hugging face", "моделі"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://huggingface.co"),
                "Відкрито Hugging Face",
            ),
            ("перплексіті", "perplexity"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://perplexity.ai"),
                "Відкрито Perplexity AI",
            ),
            ("авіасейлс", "aviasales", "купити квиток"): lambda arg: (
                self._run_and_return(
                    lambda: webbrowser.open("https://aviasales.ua"),
                    "Відкрито Aviasales",
                )
            ),
            ("букінг", "booking", "готель"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://booking.com"), "Відкрито Booking"
            ),
            ("дія", "портал дія"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://diia.gov.ua"), "Відкрито Дія"
            ),
            ("приватбанк", "privat24"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://privatbank.ua"), "Відкрито ПриватБанк"
            ),
            ("монобанк", "monobank"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://monobank.ua"), "Відкрито Монобанк"
            ),
            ("ощадбанк", "oschadbank"): lambda arg: self._run_and_return(
                lambda: webbrowser.open("https://oschadbank.ua"), "Відкрито Ощадбанк"
            ),
            ("стім", "steam", "ігри"): lambda arg: self._run_and_return(
                lambda: os.startfile("steam://open/main"), "Запущено Steam"
            ),
            ("епік геймс", "epic games"): lambda arg: self._run_and_return(
                lambda: os.startfile("com.epicgames.launcher://"),
                "Запущено Epic Games Launcher",
            ),
            ("дискорд", "discord"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(
                    [
                        os.path.expandvars(r"%localappdata%\Discord\Update.exe"),
                        "--processStart",
                        "Discord.exe",
                    ]
                ),
                "Запущено Discord",
            ),
            ("телеграм", "telegram"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(
                    [os.path.expandvars(r"%appdata%\Telegram Desktop\Telegram.exe")]
                ),
                "Запущено Telegram",
            ),
            ("спотіфай", "spotify", "музика"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(
                    [os.path.expandvars(r"%appdata%\Spotify\Spotify.exe")]
                ),
                "Запущено Spotify",
            ),
            ("відкрий ворд", "запусти word", "майкрософт ворд"): lambda arg: (
                self._run_and_return(
                    lambda: os.system("start winword"), "Відкрито Word"
                )
            ),
            ("відкрий ексель", "запусти excel", "таблиці"): lambda arg: (
                self._run_and_return(lambda: os.system("start excel"), "Відкрито Excel")
            ),
            ("відкрий презентацію", "запусти powerpoint"): lambda arg: (
                self._run_and_return(
                    lambda: os.system("start powerpnt"), "Відкрито PowerPoint"
                )
            ),
            ("вордпад", "wordpad"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["write.exe"]), "Відкрито WordPad"
            ),
            ("код", "середовище", "vs code", "віжуал студіо"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(["code"], shell=True), "Запущено VS Code"
                )
            ),
            ("термінал", "powershell", "відкрий термінал"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(["powershell.exe"]), "Відкрито PowerShell"
                )
            ),
            ("командний рядок", "cmd", "консоль"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["cmd.exe"]), "Відкрито CMD"
            ),
            ("калькулятор", "рахувати", "відкрий калькулятор"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(["calc.exe"]), "Відкрито калькулятор"
                )
            ),
            ("диспетчер завдань", "task manager", "відкрий диспетчер"): lambda arg: (
                self._run_and_return(
                    lambda: self._hotkey(0x11, 0x10, 0x1B), "Відкрито диспетчер завдань"
                )
            ),
            ("скріншот", "засіб захоплення", "ножиці", "зроби скріншот"): lambda arg: (
                self._run_and_return(
                    lambda: self._hotkey(0x5B, 0x10, 0x53), "Запущено засіб захоплення"
                )
            ),
            ("блокнот", "notepad", "відкрий блокнот"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["notepad.exe"]), "Відкрито Блокнот"
            ),
            ("пейнт", "paint", "малювати"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["mspaint.exe"]), "Відкрито Paint"
            ),
            ("календар", "відкрий календар"): lambda arg: self._run_and_return(
                lambda: os.system("start outlookcal:"), "Відкрито Календар"
            ),
            ("годинник", "будильник", "таймер"): lambda arg: self._run_and_return(
                lambda: os.system("start ms-clock:"), "Відкрито Годинник"
            ),
            ("провідник", "explorer", "відкрий папку"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(["explorer.exe"]), "Відкрито Провідник"
                )
            ),
            ("налаштування", "settings", "відкрий налаштування"): lambda arg: (
                self._run_and_return(
                    lambda: os.system("start ms-settings:"), "Відкрито Налаштування"
                )
            ),
            ("панель керування", "control panel"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["control.exe"]), "Відкрито Панель керування"
            ),
            ("камера", "фото", "відкрий камеру"): lambda arg: self._run_and_return(
                lambda: os.system("start microsoft.windows.camera:"), "Відкрито Камеру"
            ),
            ("відкрий obs", "obs студіо", "стримінг"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["obs64.exe"], shell=True),
                "Запущено OBS Studio",
            ),
            ("відкрий vlc", "vlc", "медіаплеєр"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["vlc"], shell=True), "Запущено VLC"
            ),
            ("відкрий гіт", "git bash"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["git-bash.exe"], shell=True),
                "Запущено Git Bash",
            ),
            ("відкрий intellij", "idea", "джетбрейнс"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(["idea64.exe"], shell=True),
                    "Запущено IntelliJ IDEA",
                )
            ),
            ("персоналізація", "фоновий малюнок"): lambda arg: self._run_and_return(
                lambda: os.system("start ms-settings:personalization"),
                "Відкрито Персоналізацію",
            ),
            ("блютуз", "bluetooth"): lambda arg: self._run_and_return(
                lambda: os.system("start ms-settings:bluetooth"), "Відкрито Bluetooth"
            ),
            ("налаштування мережі", "інтернет налаштування"): lambda arg: (
                self._run_and_return(
                    lambda: os.system("start ms-settings:network"),
                    "Відкрито налаштування мережі",
                )
            ),
            ("налаштування екрану", "дисплей"): lambda arg: self._run_and_return(
                lambda: os.system("start ms-settings:display"),
                "Відкрито налаштування дисплею",
            ),
            ("відкрий wifi", "налаштування вайфай", "wifi settings"): lambda arg: (
                self._run_and_return(
                    lambda: os.system("start ms-settings:network-wifi"),
                    "Відкрито налаштування Wi-Fi",
                )
            ),
            ("диспетчер пристроїв", "device manager"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["devmgmt.msc"], shell=True),
                "Відкрито Диспетчер пристроїв",
            ),
            ("редактор реєстру", "regedit"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["regedit.exe"], shell=True), "Відкрито Реєстр"
            ),
            ("служби", "services"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["services.msc"], shell=True),
                "Відкрито Служби",
            ),
            ("мережеві підключення", "інтернет з'єднання"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(["ncpa.cpl"], shell=True),
                    "Відкрито Мережеві підключення",
                )
            ),
            ("властивості системи", "параметри системи"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(["sysdm.cpl"], shell=True),
                    "Відкрито Властивості системи",
                )
            ),
            ("програми та засоби", "видалення програм"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(["appwiz.cpl"], shell=True),
                    "Відкрито Програми та засоби",
                )
            ),
            ("очищення диска", "очистити диск"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["cleanmgr.exe"], shell=True),
                "Запущено Очищення диска",
            ),
            ("екранна клавіатура", "віртуальна клавіатура"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(["osk.exe"], shell=True),
                    "Відкрито Екранну клавіатуру",
                )
            ),
            ("екранна лупа", "лупа"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["magnify.exe"], shell=True),
                "Запущено Екранну лупу",
            ),
            (
                "мій комп'ютер",
                "цей комп'ютер",
                "всі диски",
                "покажи диски",
            ): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(
                    ["explorer.exe", "shell:::{20D04FE0-3AEA-1069-A2D8-08002B30309D}"]
                ),
                "Відкрито Мій комп'ютер",
            ),
            ("завантаження", "downloads", "папка завантаження"): lambda arg: (
                self._run_and_return(
                    lambda: os.startfile(os.path.expanduser("~\\Downloads")),
                    "Відкрито Завантаження",
                )
            ),
            ("документи", "documents", "мої документи"): lambda arg: (
                self._run_and_return(
                    lambda: os.startfile(os.path.expanduser("~\\Documents")),
                    "Відкрито Документи",
                )
            ),
            ("робочий стіл", "desktop", "покажи робочий стіл"): lambda arg: (
                self._run_and_return(
                    lambda: os.startfile(os.path.expanduser("~\\Desktop")),
                    "Відкрито Робочий стіл",
                )
            ),
            (
                "диск с",
                "c drive",
                "відкрий диск с",
                "відкрий с",
                "покажи диск с",
            ): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["explorer.exe", "C:\\"]), "Відкрито Диск C"
            ),
            (
                "диск д",
                "диск d",
                "d drive",
                "відкрий диск д",
                "відкрий д",
                "покажи диск д",
            ): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["explorer.exe", "D:\\"]), "Відкрито Диск D"
            ),
            ("відкрити диск є", "диск e", "e drive", "відкрий диск е"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(["explorer.exe", "E:\\"]),
                    "Відкрито Диск E",
                )
            ),
            ("папка темп", "тимчасові файли"): lambda arg: self._run_and_return(
                lambda: os.startfile(os.environ["TEMP"]), "Відкрито папку Temp"
            ),
            ("папка аппдата", "appdata"): lambda arg: self._run_and_return(
                lambda: os.startfile(os.environ["APPDATA"]), "Відкрито папку AppData"
            ),
            ("програм файлс", "program files"): lambda arg: self._run_and_return(
                lambda: os.startfile(os.environ["ProgramFiles"]),
                "Відкрито папку Program Files",
            ),
            ("відкрий зображення", "папка зображення", "картинки"): lambda arg: (
                self._run_and_return(
                    lambda: os.startfile(os.path.expanduser("~\\Pictures")),
                    "Відкрито Зображення",
                )
            ),
            ("відкрий відео", "папка відео", "мої відео"): lambda arg: (
                self._run_and_return(
                    lambda: os.startfile(os.path.expanduser("~\\Videos")),
                    "Відкрито Відео",
                )
            ),
            ("відкрий музику", "папка музика", "мої пісні"): lambda arg: (
                self._run_and_return(
                    lambda: os.startfile(os.path.expanduser("~\\Music")),
                    "Відкрито Музику",
                )
            ),
            ("останні файли", "нещодавні файли"): lambda arg: self._run_and_return(
                lambda: os.startfile(
                    os.path.expandvars("%APPDATA%\\Microsoft\\Windows\\Recent")
                ),
                "Відкрито Останні файли",
            ),
            ("публічна папка", "спільна папка"): lambda arg: self._run_and_return(
                lambda: os.startfile("C:\\Users\\Public"), "Відкрито Публічну папку"
            ),
            ("мережеве оточення", "мережеві папки", "мережа провідник"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(
                        [
                            "explorer.exe",
                            "shell:::{F02C1A0D-BE21-4350-88B0-7367FC96EF3C}",
                        ]
                    ),
                    "Відкрито Мережеве оточення",
                )
            ),
            ("корзина", "відкрий кошик", "покажи кошик"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(
                        [
                            "explorer.exe",
                            "shell:::{645FF040-5081-101B-9F08-00AA002F954E}",
                        ]
                    ),
                    "Відкрито Кошик",
                )
            ),
            ("панель керування провідник", "всі елементи панелі"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(
                        [
                            "explorer.exe",
                            "shell:::{26EE0668-A00A-44D7-9371-BEB064C98683}",
                        ]
                    ),
                    "Відкрито Панель керування",
                )
            ),
            ("швидкий доступ", "улюблені папки"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(
                    ["explorer.exe", "shell:::{679f85cb-0220-4080-b29b-5540cc05aab6}"]
                ),
                "Відкрито Швидкий доступ",
            ),
            (
                "відкрий шлях",
                "перейди до папки",
                "відкрий директорію",
            ): self.open_explorer_path,
            (
                "знайди файл",
                "пошук файлу",
                "пошук в провіднику",
            ): self.search_in_explorer,
            (
                "нова папка на робочому столі",
                "створи папку на столі",
            ): self.create_desktop_folder,
            (
                "розмір папки завантажень",
                "скільки важать завантаження",
            ): self.get_downloads_size,
            (
                "відкрий останній завантажений",
                "останній файл завантажень",
            ): self.open_last_download,
            (
                "список файлів робочого столу",
                "що на робочому столі",
            ): self.list_desktop_files,
            ("копіювати", "скопіюй", "зроби копію"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x11, 0x43), "Скопійовано"
            ),
            ("вставити", "встав"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x11, 0x56), "Вставлено"
            ),
            ("вирізати", "виріжи"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x11, 0x58), "Вирізано"
            ),
            ("зберегти", "збережи"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x11, 0x53), "Збережено"
            ),
            ("відмінити", "поверни назад", "скасуй дію"): lambda arg: (
                self._run_and_return(lambda: self._hotkey(0x11, 0x5A), "Дію скасовано")
            ),
            ("виділити все", "виділи весь текст"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x11, 0x41), "Виділено все"
            ),
            ("нова вкладка", "відкрий нову вкладку"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x11, 0x54), "Нова вкладка"
            ),
            ("закрити вкладку", "закрий вкладку"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x11, 0x57), "Вкладку закрито"
            ),
            (
                "закрити вікно",
                "закрий програму",
                "закрий вікно",
                "альт ф4",
                "закрийся",
            ): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x12, 0x73), "Вікно закрито"
            ),
            ("буфер обміну", "історія буферу"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x5B, 0x56), "Відкрито буфер обміну"
            ),
            ("пошук віндовс", "відкрий пошук"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x5B, 0x53), "Відкрито пошук"
            ),
            ("меню пуск", "пуск"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x5B), "Відкрито Пуск"
            ),
            ("панель сповіщень", "сповіщення"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x5B, 0x41), "Відкрито сповіщення"
            ),
            ("розгорни вікно", "на весь екран"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x5B, 0x26), "Вікно розгорнуто"
            ),
            ("зменш вікно", "поверни вікно"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x5B, 0x28), "Вікно зменшено"
            ),
            ("вікно вліво", "прикріпити зліва"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x5B, 0x25), "Вікно прикріплено ліворуч"
            ),
            ("вікно вправо", "прикріпити справа"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x5B, 0x27), "Вікно прикріплено праворуч"
            ),
            ("крок назад", "попередня сторінка"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x12, 0x25), "Попередня сторінка"
            ),
            ("крок вперед", "наступна сторінка"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x12, 0x27), "Наступна сторінка"
            ),
            ("онови сторінку", "оновити"): lambda arg: self._run_and_return(
                lambda: self._press_key(0x74), "Сторінку оновлено"
            ),
            ("сторінка вгору", "прокрути вгору"): lambda arg: self._run_and_return(
                lambda: self._press_key(0x21), "Прокрутка вгору"
            ),
            ("сторінка вниз", "прокрути вниз"): lambda arg: self._run_and_return(
                lambda: self._press_key(0x22), "Прокрутка вниз"
            ),
            ("на гору", "початок сторінки"): lambda arg: self._run_and_return(
                lambda: self._press_key(0x24), "Перехід на початок"
            ),
            ("в самий низ", "кінець сторінки"): lambda arg: self._run_and_return(
                lambda: self._press_key(0x23), "Перехід в кінець"
            ),
            ("натисни ентер", "підтвердити"): lambda arg: self._run_and_return(
                lambda: self._press_key(0x0D), "Натиснуто Enter"
            ),
            ("натисни ескейп", "скасувати"): lambda arg: self._run_and_return(
                lambda: self._press_key(0x1B), "Натиснуто Escape"
            ),
            ("зітри", "видалити символ", "бекспейс"): lambda arg: self._run_and_return(
                lambda: self._press_key(0x08), "Натиснуто Backspace"
            ),
            ("видали", "натисни деліт"): lambda arg: self._run_and_return(
                lambda: self._press_key(0x2E), "Натиснуто Delete"
            ),
            ("пробіл", "натисни пробіл"): lambda arg: self._run_and_return(
                lambda: self._press_key(0x20), "Натиснуто Пробіл"
            ),
            ("друк", "роздрукуй", "надрукуй"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x11, 0x50), "Надіслано на друк"
            ),
            ("знайди в тексті", "пошук в документі", "ctrl f"): lambda arg: (
                self._run_and_return(
                    lambda: self._hotkey(0x11, 0x46), "Відкрито пошук у документі"
                )
            ),
            ("нове вікно", "відкрий нове вікно"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x11, 0x4E), "Нове вікно відкрито"
            ),
            ("повтори дію", "ctrl y"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x11, 0x59), "Дію повторено"
            ),
            ("збільш масштаб", "зум плюс"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x11, 0xBB), "Масштаб збільшено"
            ),
            ("зменш масштаб", "зум мінус"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x11, 0xBD), "Масштаб зменшено"
            ),
            ("скинь масштаб", "масштаб 100"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x11, 0x30), "Масштаб скинуто"
            ),
            ("наступне вікно", "перемикнись"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x12, 0x09), "Переключено вікно"
            ),
            ("повноекранний режим", "f11"): lambda arg: self._run_and_return(
                lambda: self._press_key(0x7A), "Повноекранний режим"
            ),
            ("відкрий інспектор", "devtools", "f12"): lambda arg: self._run_and_return(
                lambda: self._press_key(0x7B), "Відкрито DevTools"
            ),
            ("фокус адресний рядок", "перейди в адрес", "ctrl l"): lambda arg: (
                self._run_and_return(
                    lambda: self._hotkey(0x11, 0x4C), "Фокус у адресному рядку"
                )
            ),
            ("гучніше", "збільш звук", "голосніше", "додай звук"): lambda arg: (
                self._run_and_return(lambda: self._press_key(0xAF, 5), "Звук збільшено")
            ),
            ("тихіше", "зменш звук", "зроби тихіше"): lambda arg: self._run_and_return(
                lambda: self._press_key(0xAE, 5), "Звук зменшено"
            ),
            ("вимкни звук", "без звуку", "м'ют", "заглуши"): lambda arg: (
                self._run_and_return(lambda: self._press_key(0xAD), "Звук вимкнено")
            ),
            ("наступний трек", "наступна пісня", "включи наступне"): lambda arg: (
                self._run_and_return(lambda: self._press_key(0xB0), "Наступний трек")
            ),
            ("попередній трек", "попередня пісня", "включи попереднє"): lambda arg: (
                self._run_and_return(lambda: self._press_key(0xB1), "Попередній трек")
            ),
            (
                "пауза",
                "продовжити музику",
                "стоп музика",
                "грати музику",
                "зупини відео",
            ): lambda arg: self._run_and_return(
                lambda: self._press_key(0xB3), "Пауза/Відтворення"
            ),
            ("згорни вікна", "згорнути всі вікна", "сховай все"): lambda arg: (
                self._run_and_return(self._minimize_all, "Вікна згорнуто")
            ),
            ("пінг", "перевір з'єднання", "перевір інтернет"): self.ping_google,
            ("скинь dns", "очисти dns", "dns flush"): lambda arg: self._run_and_return(
                lambda: os.system("ipconfig /flushdns"), "DNS кеш очищено"
            ),
            ("оновити ip", "ipconfig renew"): lambda arg: self._run_and_return(
                lambda: os.system("ipconfig /release && ipconfig /renew"),
                "IP адресу оновлено",
            ),
            ("мережева статистика", "netstat"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["cmd.exe", "/k", "netstat -an"]),
                "Відкрито netstat",
            ),
            ("перезапустити провідник", "рестарт експлорер"): lambda arg: (
                self._run_and_return(
                    lambda: os.system(
                        "taskkill /f /im explorer.exe & start explorer.exe"
                    ),
                    "Провідник перезапущено",
                )
            ),
            ("очистити буфер", "видали буфер"): lambda arg: self._run_and_return(
                lambda: os.system("echo off | clip"), "Буфер обміну очищено"
            ),
            ("новий робочий стіл", "створи робочий стіл"): lambda arg: (
                self._run_and_return(
                    lambda: self._hotkey(0x5B, 0x11, 0x44),
                    "Створено новий робочий стіл",
                )
            ),
            ("закрити робочий стіл", "видали робочий стіл"): lambda arg: (
                self._run_and_return(
                    lambda: self._hotkey(0x5B, 0x11, 0x73), "Робочий стіл закрито"
                )
            ),
            ("наступний робочий стіл", "перемикни вперед"): lambda arg: (
                self._run_and_return(
                    lambda: self._hotkey(0x5B, 0x11, 0x27), "Наступний робочий стіл"
                )
            ),
            ("попередній робочий стіл", "перемикни назад"): lambda arg: (
                self._run_and_return(
                    lambda: self._hotkey(0x5B, 0x11, 0x25), "Попередній робочий стіл"
                )
            ),
            ("подання завдань", "усі вікна", "task view"): lambda arg: (
                self._run_and_return(
                    lambda: self._hotkey(0x5B, 0x09), "Відкрито подання завдань"
                )
            ),
            ("меню проектування", "другий екран", "дублювати екран"): lambda arg: (
                self._run_and_return(
                    lambda: self._hotkey(0x5B, 0x50), "Відкрито меню проектування"
                )
            ),
            (
                "очисти кошик",
                "чистий кошик",
                "видали сміття",
                "почисть кошик",
            ): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(
                    [
                        "powershell.exe",
                        "-Command",
                        "Clear-RecycleBin -Force -ErrorAction SilentlyContinue",
                    ]
                ),
                "Кошик очищено",
            ),
            ("заблокуй", "заблокувати екран", "йду геть"): lambda arg: (
                self._run_and_return(
                    lambda: os.system("rundll32.exe user32.dll,LockWorkStation"),
                    "Екран заблоковано",
                )
            ),
            ("сплячий режим", "режим сну"): lambda arg: self._run_and_return(
                lambda: os.system("rundll32.exe powrprof.dll,SetSuspendState 0,1,0"),
                "Перехід у сплячий режим",
            ),
            ("гібернація", "режим гібернації"): lambda arg: self._run_and_return(
                lambda: os.system("shutdown /h"), "Перехід у гібернацію"
            ),
            ("вимкни комп'ютер", "завершення роботи", "вирубай комп"): lambda arg: (
                self._run_and_return(
                    lambda: os.system("shutdown /s /t 60"), "Вимкнення через 60с"
                )
            ),
            ("перезавантаж", "рестарт", "перезапусти комп"): lambda arg: (
                self._run_and_return(
                    lambda: os.system("shutdown /r /t 60"), "Перезавантаження через 60с"
                )
            ),
            ("скасуй вимкнення", "відміни вимкнення", "зупини рестарт"): lambda arg: (
                self._run_and_return(lambda: os.system("shutdown /a"), "Скасовано")
            ),
            ("вийти з системи", "логаут"): lambda arg: self._run_and_return(
                lambda: os.system("logoff"), "Вихід з системи"
            ),
            ("термінал адміністратора", "консоль адміністратора"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(
                        ["powershell.exe", "-Command", "Start-Process cmd -Verb RunAs"]
                    ),
                    "Відкрито термінал з правами адміністратора",
                )
            ),
            ("монітор ресурсів", "завантаженість системи"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(["resmon.exe"]),
                    "Відкрито Монітор ресурсів",
                )
            ),
            ("керування дисками", "менеджер дисків"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["diskmgmt.msc"], shell=True),
                "Відкрито Керування дисками",
            ),
            ("перегляд подій", "журнал подій"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["eventvwr.msc"], shell=True),
                "Відкрито Перегляд подій",
            ),
            ("планувальник завдань", "планувальник"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["taskschd.msc"], shell=True),
                "Відкрито Планувальник завдань",
            ),
            ("редактор політики", "групова політика"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["gpedit.msc"], shell=True),
                "Відкрито Редактор локальної політики",
            ),
            ("конфігурація системи", "мс конфіг"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["msconfig.exe"]),
                "Відкрито Конфігурацію системи",
            ),
            ("діагностика", "директ ікс"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["dxdiag.exe"]),
                "Відкрито засіб діагностики DirectX",
            ),
            ("системна інформація", "інфо про систему"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(["msinfo32.exe"]),
                    "Відкрито Відомості про систему",
                )
            ),
            ("облікові записи", "користувачі віндовс"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(["netplwiz.exe"]),
                    "Відкрито Облікові записи",
                )
            ),
            ("властивості миші", "налаштування миші"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["main.cpl"], shell=True),
                "Відкрито властивості миші",
            ),
            ("звукові пристрої", "налаштування звуку"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(["mmsys.cpl"], shell=True),
                    "Відкрито налаштування звуку",
                )
            ),
            ("дата і час", "налаштування часу"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["timedate.cpl"], shell=True),
                "Відкрито налаштування часу і дати",
            ),
            ("центр оновлень", "оновлення віндовс"): lambda arg: self._run_and_return(
                lambda: os.system("start ms-settings:windowsupdate"),
                "Відкрито Центр оновлень",
            ),
            ("брандмауер", "фаєрвол"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["firewall.cpl"], shell=True),
                "Відкрито Брандмауер",
            ),
            ("антивірус", "захисник віндовс"): lambda arg: self._run_and_return(
                lambda: os.system("start windowsdefender:"), "Відкрито Захисник Windows"
            ),
            ("принтери", "пристрої та принтери"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["control", "printers"], shell=True),
                "Відкрито Пристрої та принтери",
            ),
            ("час", "година", "годин", "котра година"): self.get_time,
            ("дата", "число", "яке сьогодні число"): self.get_date,
            ("день тижня", "який день"): self.get_day_of_week,
            ("скільки часу до нового року", "нового року"): self.get_days_to_new_year,
            ("яке сьогодні свято", "свято сьогодні"): self.get_today_holiday,
            ("оперативна пам'ять", "скільки рам", "пам'ять системи"): self.get_ram_info,
            ("процесор", "завантаженість процесора", "скільки цпу"): self.get_cpu_info,
            ("диск", "місце на диску", "вільне місце"): self.get_disk_info,
            ("ip адреса", "моя ip", "айпі"): self.get_ip_address,
            ("мак адреса", "mac адреса"): self.get_mac_address,
            ("ім'я комп'ютера", "хостнейм", "назва пк"): self.get_hostname,
            (
                "версія системи",
                "яка операційна система",
                "ос версія",
            ): self.get_os_version,
            ("час роботи", "аптайм", "скільки працює"): self.get_uptime,
            ("батарея", "заряд батареї", "акумулятор"): self.get_battery,
            (
                "запущені програми",
                "список процесів",
                "що запущено",
            ): self.get_running_apps,
            ("рандомне число", "випадкове число"): self.get_random_number,
            ("монетка", "орел чи решка", "підкинь монетку"): self.flip_coin,
            ("кубик", "кинь кубик", "рандом кубик"): self.roll_dice,
            ("пароль", "згенеруй пароль", "новий пароль"): self.generate_password,
            ("порахуй", "обчисли", "скільки буде"): self.calculate,
            ("анекдот", "розсміши мене", "жарт"): self.tell_joke,
            ("факт про програмування", "цікавий факт"): self.tell_fact,
            (
                "мотивація",
                "мотивацію",
                "надихни мене",
                "цитата",
                "цитату",
            ): self.tell_motivation,
            ("вимкни екран", "погаси монітор", "вимкни дисплей"): self.turn_off_display,
            (
                "очистити темп",
                "видалити тимчасові файли",
                "почисть temp",
            ): self.clear_temp_files,
            ("темна тема", "увімкни темну тему"): lambda arg: self.set_windows_theme(
                "dark"
            ),
            ("світла тема", "увімкни світлу тему"): lambda arg: self.set_windows_theme(
                "light"
            ),
            (
                "вбий завислі",
                "заверши завислі програми",
                "очисти завислі",
            ): self.kill_hung_tasks,
            (
                "приховані файли",
                "покажи приховані",
                "сховай файли",
            ): self.toggle_hidden_files,
            ("матриця", "ефект матриці", "режим хакера"): self.run_matrix,
            ("пароль вайфай", "пароль від мережі"): self.get_wifi_password,
            ("швидка нотатка", "нотатка", "запиши", "занотуй"): self.quick_note,
            ("нічне світло", "режим читання", "захист очей"): lambda arg: (
                self._run_and_return(
                    lambda: os.system("start ms-settings:nightlight"),
                    "Відкрито Нічне світло",
                )
            ),
            ("автозавантаження", "програми при запуску"): lambda arg: (
                self._run_and_return(
                    lambda: os.system("start ms-settings:startupapps"),
                    "Відкрито Автозавантаження",
                )
            ),
            ("змінні середовища", "environment variables"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(
                        ["rundll32.exe", "sysdm.cpl,EditEnvironmentVariables"]
                    ),
                    "Відкрито Змінні середовища",
                )
            ),
            ("файл хостс", "відкрий hosts"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(
                    ["notepad.exe", r"C:\Windows\System32\drivers\etc\hosts"]
                ),
                "Відкрито файл hosts",
            ),
            ("запис екрану", "почни запис відео"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x5B, 0x12, 0x52),
                "Перемикання запису екрану (Win+Alt+R)",
            ),
            ("ігрова панель", "game bar"): lambda arg: self._run_and_return(
                lambda: self._hotkey(0x5B, 0x47), "Відкрито Xbox Game Bar"
            ),
            ("налаштування буфера", "історія буфера налаштування"): lambda arg: (
                self._run_and_return(
                    lambda: os.system("start ms-settings:clipboard"),
                    "Налаштування буфера обміну",
                )
            ),
            ("налаштування vpn", "відкрий vpn"): lambda arg: self._run_and_return(
                lambda: os.system("start ms-settings:network-vpn"),
                "Відкрито налаштування VPN",
            ),
            ("компоненти віндовс", "windows features"): lambda arg: (
                self._run_and_return(
                    lambda: subprocess.Popen(["optionalfeatures.exe"]),
                    "Відкрито Компоненти Windows",
                )
            ),
            ("параметри папок", "властивості папки"): lambda arg: self._run_and_return(
                lambda: subprocess.Popen(["control.exe", "folders"]),
                "Відкрито Параметри папок",
            ),
            ("згенеруй guid", "згенеруй uuid", "новий id"): self.generate_uuid,
            (
                "випадковий колір",
                "рандомний колір",
                "колір hex",
            ): self.generate_hex_color,
            ("переверни текст", "реверс тексту"): self.reverse_text,
            ("перевір сайт", "статус сайту", "сайт працює"): self.check_site_status,
        }

        self.commands = {}
        for triggers, action in raw_commands.items():
            for t in triggers:
                self.commands[t] = action

    def execute(self, text: str) -> str:
        text_lower = text.lower()
        for trigger in sorted(self.commands.keys(), key=len, reverse=True):
            idx = text_lower.find(trigger)
            if idx != -1:
                arg = text_lower[idx + len(trigger) :].strip()
                return self.commands[trigger](arg)
        return ""

    def _run_and_return(self, func, msg: str) -> str:
        try:
            func()
        except Exception:
            pass
        return msg

    def _press_key(self, vk_code: int, times: int = 1):
        for _ in range(times):
            ctypes.windll.user32.keybd_event(vk_code, 0, 0, 0)
            ctypes.windll.user32.keybd_event(vk_code, 0, 2, 0)

    def _hotkey(self, *keys):
        for key in keys:
            ctypes.windll.user32.keybd_event(key, 0, 0, 0)
        for key in reversed(keys):
            ctypes.windll.user32.keybd_event(key, 0, 2, 0)

    def _minimize_all(self):
        self._hotkey(0x5B, 0x4D)

    def open_website(self, arg: str) -> str:
        if not arg:
            return ""
        url = arg.replace(" ", "")
        if not url.startswith("http"):
            url = f"https://{url}"
        webbrowser.open(url)
        return f"Відкриваю сайт: {url}"

    def search_google(self, arg: str) -> str:
        if not arg:
            return ""
        webbrowser.open(f"https://www.google.com/search?q={arg}")
        return f"Шукаю в Google: {arg}"

    def search_wikipedia(self, arg: str) -> str:
        if not arg:
            return ""
        webbrowser.open(f"https://uk.wikipedia.org/wiki/{arg.replace(' ', '_')}")
        return f"Шукаю у Вікіпедії: {arg}"

    def search_stackoverflow(self, arg: str) -> str:
        if not arg:
            return ""
        webbrowser.open(f"https://stackoverflow.com/search?q={arg}")
        return f"Шукаю на StackOverflow: {arg}"

    def search_youtube(self, arg: str) -> str:
        if not arg:
            return ""
        webbrowser.open(
            f"https://www.youtube.com/results?search_query={arg.replace(' ', '+')}"
        )
        return f"Шукаю на YouTube: {arg}"

    def search_github(self, arg: str) -> str:
        if not arg:
            return ""
        webbrowser.open(f"https://github.com/search?q={arg.replace(' ', '+')}")
        return f"Шукаю на GitHub: {arg}"

    def search_npm(self, arg: str) -> str:
        if not arg:
            return ""
        webbrowser.open(f"https://www.npmjs.com/search?q={arg.replace(' ', '+')}")
        return f"Шукаю пакет на npm: {arg}"

    def search_pypi(self, arg: str) -> str:
        if not arg:
            return ""
        webbrowser.open(f"https://pypi.org/search/?q={arg.replace(' ', '+')}")
        return f"Шукаю пакет на PyPI: {arg}"

    def search_google_images(self, arg: str) -> str:
        if not arg:
            return ""
        webbrowser.open(
            f"https://www.google.com/search?tbm=isch&q={arg.replace(' ', '+')}"
        )
        return f"Шукаю зображення: {arg}"

    def search_maps(self, arg: str) -> str:
        if not arg:
            return ""
        webbrowser.open(f"https://www.google.com/maps/search/{arg.replace(' ', '+')}")
        return f"Шукаю на картах: {arg}"

    def search_news(self, arg: str) -> str:
        if not arg:
            webbrowser.open("https://ukr.net")
            return "Відкрито новини"
        webbrowser.open(
            f"https://www.google.com/search?q={arg.replace(' ', '+')}+новини&tbm=nws"
        )
        return f"Шукаю новини: {arg}"

    def search_dictionary(self, arg: str) -> str:
        if not arg:
            return ""
        webbrowser.open(f"https://uk.wiktionary.org/wiki/{arg.replace(' ', '_')}")
        return f"Тлумачення: {arg}"

    def translate_word(self, arg: str) -> str:
        if not arg:
            webbrowser.open("https://translate.google.com")
            return "Відкрито перекладач"
        webbrowser.open(
            f"https://translate.google.com/?sl=auto&tl=uk&text={arg.replace(' ', '+')}&op=translate"
        )
        return f"Перекладаю: {arg}"

    def get_time(self, arg: str) -> str:
        return f"Поточний час: {datetime.datetime.now().strftime('%H:%M')}"

    def get_date(self, arg: str) -> str:
        return f"Сьогоднішня дата: {datetime.datetime.now().strftime('%d.%m.%Y')}"

    def get_day_of_week(self, arg: str) -> str:
        days = [
            "Понеділок",
            "Вівторок",
            "Середа",
            "Четвер",
            "П'ятниця",
            "Субота",
            "Неділя",
        ]
        return f"Сьогодні: {days[datetime.datetime.now().weekday()]}"

    def get_days_to_new_year(self, arg: str) -> str:
        now = datetime.datetime.now()
        ny = datetime.datetime(now.year + 1, 1, 1)
        return f"До Нового року залишилось: {(ny - now).days} днів"

    def get_today_holiday(self, arg: str) -> str:
        today = datetime.datetime.now()
        holidays = {
            (1, 1): "Новий рік",
            (1, 7): "Різдво (юліанський)",
            (2, 14): "День закоханих",
            (3, 8): "Міжнародний жіночий день",
            (5, 1): "День праці",
            (5, 9): "День перемоги над нацизмом",
            (6, 1): "День захисту дітей",
            (6, 28): "День Конституції України",
            (8, 24): "День Незалежності України",
            (10, 14): "День захисника та захисниці України",
            (12, 19): "День Святого Миколая",
            (12, 25): "Різдво (григоріанський)",
        }
        key = (today.month, today.day)
        if key in holidays:
            return f"Сьогодні свято: {holidays[key]}!"
        return "Сьогодні немає офіційного свята"

    def get_ram_info(self, arg: str) -> str:
        if psutil is None:
            return "Встановіть psutil: pip install psutil"
        mem = psutil.virtual_memory()
        return (
            f"RAM: {mem.used // (1024**3)} ГБ з "
            f"{mem.total // (1024**3)} ГБ ({mem.percent}%)"
        )

    def get_cpu_info(self, arg: str) -> str:
        if psutil is None:
            return "Встановіть psutil: pip install psutil"
        cpu = psutil.cpu_percent(interval=0.5)
        cores = psutil.cpu_count(logical=False)
        threads = psutil.cpu_count(logical=True)
        return f"CPU: {cpu}% | {cores} ядра / {threads} потоки"

    def get_disk_info(self, arg: str) -> str:
        if psutil is None:
            return "Встановіть psutil: pip install psutil"
        disk = psutil.disk_usage("C:\\")
        return (
            f"Диск C: {disk.used // (1024**3)} ГБ зайнято, "
            f"{disk.free // (1024**3)} ГБ вільно з {disk.total // (1024**3)} ГБ"
        )

    def get_ip_address(self, arg: str) -> str:
        try:
            return f"Локальна IP: {socket.gethostbyname(socket.gethostname())}"
        except Exception:
            return "Не вдалося отримати IP"

    def get_mac_address(self, arg: str) -> str:
        try:
            import uuid

            mac = ":".join(
                [
                    "{:02x}".format((uuid.getnode() >> ele) & 0xFF)
                    for ele in range(0, 48, 8)
                ][::-1]
            )
            return f"MAC адреса: {mac}"
        except Exception:
            return "Не вдалося отримати MAC"

    def get_hostname(self, arg: str) -> str:
        return f"Ім'я комп'ютера: {socket.gethostname()}"

    def get_os_version(self, arg: str) -> str:
        return f"ОС: {platform.system()} {platform.release()} ({platform.version()})"

    def get_uptime(self, arg: str) -> str:
        if psutil is None:
            return "Встановіть psutil: pip install psutil"
        delta = datetime.datetime.now() - datetime.datetime.fromtimestamp(
            psutil.boot_time()
        )
        h, rem = divmod(int(delta.total_seconds()), 3600)
        m = rem // 60
        return f"Система працює: {h} год {m} хв"

    def get_battery(self, arg: str) -> str:
        if psutil is None:
            return "Встановіть psutil: pip install psutil"
        b = psutil.sensors_battery()
        if b is None:
            return "Батарея не виявлена"
        status = "заряджається" if b.power_plugged else "розряджається"
        return f"Батарея: {b.percent:.0f}% ({status})"

    def get_running_apps(self, arg: str) -> str:
        if psutil is None:
            return "Встановіть psutil: pip install psutil"
        names = list(
            dict.fromkeys(
                p.name() for p in psutil.process_iter(["name"]) if p.info["name"]
            )
        )[:10]
        return "Запущено: " + ", ".join(names)

    def ping_google(self, arg: str) -> str:
        host = arg.strip() if arg.strip() else "google.com"
        result = subprocess.run(
            ["ping", "-n", "3", host], capture_output=True, text=True, encoding="cp866"
        )
        if "TTL=" in result.stdout or "ttl=" in result.stdout:
            return f"З'єднання з {host}: OK ✔"
        return f"Немає відповіді від {host} ✘"

    def get_random_number(self, arg: str) -> str:
        try:
            parts = arg.split()
            a, b = (int(parts[0]), int(parts[1])) if len(parts) >= 2 else (1, 100)
        except (ValueError, IndexError):
            a, b = 1, 100
        return f"Випадкове число від {a} до {b}: {random.randint(a, b)}"

    def flip_coin(self, arg: str) -> str:
        return f"Монетка: {random.choice(['Орел', 'Решка'])}"

    def roll_dice(self, arg: str) -> str:
        return f"Кубик: {random.randint(1, 6)}"

    def generate_password(self, arg: str) -> str:
        import string

        length = int(arg.strip()) if arg.strip().isdigit() else 16
        chars = string.ascii_letters + string.digits + "!@#$%^&*()"
        pwd = "".join(random.choices(chars, k=length))

        if pyperclip is not None:
            pyperclip.copy(pwd)
            return f"Пароль скопійовано: {pwd}"

        return f"Пароль: {pwd}"

    def calculate(self, arg: str) -> str:
        if not arg:
            return "Вкажіть вираз, наприклад: 'порахуй 2 плюс 2'"
        expr = (
            arg.replace("плюс", "+")
            .replace("мінус", "-")
            .replace("помножити на", "*")
            .replace("ділити на", "/")
            .replace("в степені", "**")
            .replace("квадрат", "**2")
            .replace(",", ".")
        )
        try:
            result = eval(expr, {"__builtins__": {}})
            return f"Результат: {result}"
        except Exception:
            return f"Не вдалося обчислити: {arg}"

    def tell_joke(self, arg: str) -> str:
        joke = self.jokes[self.joke_idx]
        self.joke_idx = (self.joke_idx + 1) % len(self.jokes)
        return joke

    def tell_fact(self, arg: str) -> str:
        fact = self.facts[self.fact_idx]
        self.fact_idx = (self.fact_idx + 1) % len(self.facts)
        return fact

    def tell_motivation(self, arg: str) -> str:
        quote = self.quotes[self.quote_idx]
        self.quote_idx = (self.quote_idx + 1) % len(self.quotes)
        return quote

    def open_explorer_path(self, arg: str) -> str:
        if not arg:
            subprocess.Popen(["explorer.exe"])
            return "Відкрито Провідник"
        path = os.path.expandvars(os.path.expanduser(arg))
        if os.path.exists(path):
            subprocess.Popen(["explorer.exe", path])
            return f"Відкрито: {path}"
        return f"Шлях не знайдено: {path}"

    def search_in_explorer(self, arg: str) -> str:
        if not arg:
            return "Вкажіть ім'я файлу для пошуку"
        subprocess.Popen(
            ["explorer.exe", f"search-ms:query={arg}&crumb=location:C%3A%5C"]
        )
        return f"Шукаю файл: {arg}"

    def create_desktop_folder(self, arg: str) -> str:
        name = arg.strip() if arg.strip() else "Нова папка"
        path = os.path.join(os.path.expanduser("~\\Desktop"), name)
        try:
            os.makedirs(path, exist_ok=True)
            subprocess.Popen(["explorer.exe", path])
            return f"Папку створено і відкрито: {name}"
        except Exception as e:
            return f"Помилка створення папки: {e}"

    def get_downloads_size(self, arg: str) -> str:
        folder = os.path.expanduser("~\\Downloads")
        total = sum(
            os.path.getsize(os.path.join(dp, f))
            for dp, _, files in os.walk(folder)
            for f in files
        )
        mb = total / (1024**2)
        gb = mb / 1024
        size_str = f"{gb:.2f} ГБ" if gb >= 1 else f"{mb:.1f} МБ"
        return f"Папка Завантаження важить: {size_str}"

    def open_last_download(self, arg: str) -> str:
        folder = os.path.expanduser("~\\Downloads")
        try:
            files = [os.path.join(folder, f) for f in os.listdir(folder)]
            files = [f for f in files if os.path.isfile(f)]
            if not files:
                return "Папка завантажень порожня"
            latest = max(files, key=os.path.getmtime)
            os.startfile(latest)
            return f"Відкрито: {os.path.basename(latest)}"
        except Exception as e:
            return f"Помилка: {e}"

    def list_desktop_files(self, arg: str) -> str:
        folder = os.path.expanduser("~\\Desktop")
        try:
            items = os.listdir(folder)
            if not items:
                return "Робочий стіл порожній"
            return "На столі: " + ", ".join(items[:12])
        except Exception as e:
            return f"Помилка: {e}"

    def turn_off_display(self, arg: str) -> str:
        ctypes.windll.user32.SendMessageW(0xFFFF, 0x0112, 0xF170, 2)
        return "Екран вимкнено"

    def clear_temp_files(self, arg: str) -> str:

        temp_path = os.environ.get("TEMP")

        if not temp_path:
            return "Помилка: Змінну середовища TEMP не знайдено"
        count = 0
        for item in os.listdir(temp_path):
            item_path = os.path.join(temp_path, item)
            try:
                if os.path.isfile(item_path) or os.path.islink(item_path):
                    os.unlink(item_path)
                elif os.path.isdir(item_path):
                    shutil.rmtree(item_path)
                count += 1
            except Exception:
                pass
        return f"Очищено {count} об'єктів з папки Temp"

    def set_windows_theme(self, theme: str) -> str:
        val = 0 if theme == "dark" else 1
        cmd = f"reg add HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize /v AppsUseLightTheme /t REG_DWORD /d {val} /f"
        subprocess.run(cmd, shell=True, capture_output=True)
        return f"Увімкнено {'темну' if theme == 'dark' else 'світлу'} тему"

    def kill_hung_tasks(self, arg: str) -> str:
        result = subprocess.run(
            ["taskkill", "/F", "/FI", "STATUS eq NOT RESPONDING"],
            capture_output=True,
            text=True,
            encoding="cp866",
        )
        if "Успешно" in result.stdout or "SUCCESS" in result.stdout:
            return "Всі завислі процеси успішно завершено"
        return "Завислих процесів не знайдено"

    def toggle_hidden_files(self, arg: str) -> str:
        import winreg

        try:
            key = winreg.OpenKey(
                winreg.HKEY_CURRENT_USER,
                r"Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced",
                0,
                winreg.KEY_ALL_ACCESS,
            )
            value, _ = winreg.QueryValueEx(key, "Hidden")
            new_value = 1 if value == 2 else 2  # 1 = показувати, 2 = приховувати
            winreg.SetValueEx(key, "Hidden", 0, winreg.REG_DWORD, new_value)
            winreg.CloseKey(key)

            os.system("taskkill /f /im explorer.exe & start explorer.exe")
            state = "увімкнено" if new_value == 1 else "вимкнено"
            return f"Відображення прихованих файлів {state}"
        except Exception as e:
            return f"Помилка доступу до реєстру: {e}"

    def run_matrix(self, arg: str) -> str:
        bat_path = os.path.join(os.environ.get("TEMP", "C:\\"), "matrix.bat")
        with open(bat_path, "w", encoding="utf-8") as f:
            f.write(
                "@echo off\ncolor 0A\n:a\necho %random%%random%%random%%random%%random%%random%%random%%random%\ngoto a"
            )
        subprocess.Popen(["cmd.exe", "/c", f"start {bat_path}"])
        return "Wake up, Neo..."

    def get_wifi_password(self, arg: str) -> str:
        if not arg:
            return "Вкажіть назву мережі, наприклад: 'пароль вайфай my_home'"

        result = subprocess.run(
            ["netsh", "wlan", "show", "profile", f"name={arg}", "key=clear"],
            capture_output=True,
            text=True,
            encoding="cp866",
        )

        for line in result.stdout.split("\n"):
            if "Содержимое ключа" in line or "Key Content" in line:
                pwd = line.split(":")[-1].strip()
                return f"Пароль від {arg}: {pwd}"

        return f"Мережу {arg} не знайдено, або пароль відсутній"

    def quick_note(self, arg: str) -> str:
        if not arg:
            return "Що саме записати?"

        path = os.path.join(os.path.expanduser("~\\Desktop"), "Нотатка.txt")
        timestamp = datetime.datetime.now().strftime("%d.%m %H:%M")

        try:
            with open(path, "a", encoding="utf-8") as f:
                f.write(f"[{timestamp}] {arg}\n")
            return "Нотатку збережено на робочому столі"
        except Exception as e:
            return f"Помилка запису: {e}"

    def generate_uuid(self, arg: str) -> str:
        import uuid

        new_id = str(uuid.uuid4())
        if pyperclip is not None:
            pyperclip.copy(new_id)
            return f"UUID скопійовано: {new_id}"
        return f"Згенерований UUID: {new_id}"

    def generate_hex_color(self, arg: str) -> str:
        color = "#{:06x}".format(random.randint(0, 0xFFFFFF))
        if pyperclip is not None:
            pyperclip.copy(color)
            return f"Колір {color} згенеровано і скопійовано"
        return f"Випадковий колір: {color}"

    def reverse_text(self, arg: str) -> str:
        if not arg:
            return "Немає тексту для перевертання. Скажіть, наприклад: переверни текст привіт"
        rev = arg[::-1]
        if pyperclip is not None:
            pyperclip.copy(rev)
            return f"Текст перевернуто і скопійовано: {rev}"
        return f"Результат: {rev}"

    def encode_base64(self, arg: str) -> str:
        if not arg:
            return "Вкажіть текст для кодування"
        import base64

        encoded = base64.b64encode(arg.encode("utf-8")).decode("utf-8")
        if pyperclip is not None:
            pyperclip.copy(encoded)
            return f"Закодовано та скопійовано: {encoded}"
        return f"Результат Base64: {encoded}"

    def check_site_status(self, arg: str) -> str:
        if not arg:
            return "Вкажіть сайт, наприклад: 'перевір сайт google.com'"
        url = arg.strip()
        if not url.startswith("http"):
            url = "https://" + url

        import urllib.request

        try:
            code = urllib.request.urlopen(url, timeout=5).getcode()
            if code == 200:
                return f"Сайт працює (200 OK)"
            return f"Сайт повернув код {code}"
        except Exception:
            return f"Сайт недоступний або сталася помилка"

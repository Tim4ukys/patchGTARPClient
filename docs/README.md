[![GitHub license](https://img.shields.io/github/license/Tim4ukys/patchGTARPClient)](https://github.com/Tim4ukys/patchGTARPClient/blob/main/LICENSE)
[![GitHub top language](https://img.shields.io/github/languages/top/Tim4ukys/patchGTARPClient)](https://github.com/Tim4ukys/patchGTARPClient/search?l=c%2B%2B)
[![GitHub tag (latest by date)](https://img.shields.io/github/v/tag/Tim4ukys/patchGTARPClient?label=version)](https://github.com/Tim4ukys/patchGTARPClient/releases/latest)
[![GitHub Release Date](https://img.shields.io/github/release-date/Tim4ukys/patchGTARPClient)](https://github.com/Tim4ukys/patchGTARPClient/releases)
[![GitHub download](https://img.shields.io/github/downloads/tim4ukys/patchGTARPClient/total.svg?label=Total%20download)](https://github.com/Tim4ukys/patchGTARPClient/releases/latest)

## Patch GTA RP

__Описание:__ ASI Plugin который патчит(латает)/улучшает клиент GTA RP.

---
## Установка

*Фотография кликабельная. При нажатии на неё перекинет на видео на YouTube*</br>
[![Видео](https://img.youtube.com/vi/YC9oGw_tunQ/0.jpg)](https://www.youtube.com/watch?v=YC9oGw_tunQ)


Чтобы установить последнию версию данного плагина, перейдите в [_Release_](https://github.com/Tim4ukys/patchGTARPClient/releases/latest) и скачайте _patchGTARPClientByTim4ukys.zip_.</br> 
В этом архиве будет лежать два файла, Вам нужен именно _!000patchGTARPClientByTim4ukys.ASI_.</br>
Этот файл нужно будет перенести в корневую папку игры *(эта та папка в которой лежит gta_sa.exe)*.

Так же, если Вы хотите, чтобы новая версия плагина автоматически скачивалась при заходе в игру, в корневую папку с игрой перетащите *updater_patchGTARPclient.exe*.</br>
Когда выйдет обновление патча, он закроет игру и скачает обновление. После того, как он установит обновление, нужно запустить игру ещё раз.

---
## Настройки

Для настройки работы плагина в этом моде используется JSON. Ниже предоставлен конфиг с комментариями, _"какой параметр и что он делает"_.</br>
Сам конфиг должен лежить в папке с игрой как _"!000patchGTARPClientByTim4ukys.json"_. При первом запуске игры и её закрытии с этим модом, он автоматически создатся в папке с параметрами по умолчанию.

__P.S.__ С обновлением [`8.1.0`](https://github.com/Tim4ukys/patchGTARPClient/releases/tag/v8.1.0) настраивать JSON конфиг можно прям в игре(cmd: /patch_open).

```json5

{
    /* То, что связано с часами на худе(в верхнем правом угле) */
    "clock": {
        /*
            На часах будет время по МСК(коректно работает только если на ПК установлен верный часовой пояс).

            true - время по МСК.
            false - то время, которое установлено на ПК. 
        */
        "fixTimeZone": true
    },
    /* Всё, что связанно на 100% с gta sa */
    "gtasa": {
        /*
            "Turn-*******-radio-off" - мод, который делает так, чтобы при посадке в автомобиль, радиостанция
            был автоматически выключена.
            
            true - да, включить этот мод
            false - нет, отсавить всё как есть
        */
        "tfro": true
    },
    /* Всё, что связоно со старым худом */
    "oldHud": {
        /*
            Возращает старый худ.

            true - Да, пускай будет старый.
            false - нет, меня полностью устраивает тот худ, который сейчас на GTA RP.
        */
        "hud": true,
        /*
            Путь до hud.txd

            "NONE" - (по умолчанию), ничего не меняет, оставляет всё как есть.
            "Любой другой путь" - заменяет путь на этот, который показан внизу.
        */
        "pathToTXDhud": "%s\\CustomSAA2\\hud.txd",
        /*
            Возращает старый радар.

            true - Да, вернуть старый радар.
            false - Нет, меня устраивает новый.
        */
        "radar": true,
        "radarScaleFix": true
    },
    /* Всё, что связано с сампусиком(SA-MP :D) */
    "samp": {
        /*
            Заменяет шрифт чата на другой(на тот, который написан в "fontFaceName").

            true - заменить шрифт на свой.
            false - оставить всё как есть. 
        */
        "isCustomFont": true,
        /*
            Название шрифта.
            P.s. Этот шрифт должен быть устоновлен в системе!
        */
        "fontFaceName": "Comic Sans MS",
        /*
            Ускоряет скорость создания скриншота до 0.01 - 0.02 секунд(зависит от скорости RAM памяти).
            P.s. Из-за схожей скорости работы создания скриншота с клиентом Steam, по созданию скриншота
            будет слышан характерный звук.
            
            true - да, я хочу чтобы скриншоты создавались быстро
            false - нет, я хочу чтобы они создавались так же долго, как и в обычном SA-MP
        */
        "isMakeQuickScreenshot": true,
        /*
            Изменяет формат сохраняемых скриншотов.
           JPEG - самый маленький размер, качество на любителя
           PNG - средний размер, качество нормальное
           TGA - большой размер, качество отличное
        */
        "formatScreenshotIMG": "JPEG",
        /*
            После создания скриншота проигрывает звук. 
            P.s. Работает только если isMakeQuickScreenshot имеет значение true

            true - да, я хочу чтобы после создания скриншота был некий звук
            false - нет, я хочу чтобы никакого доп. звука не было
        */
        "isPlaySoundAfterMakeScreenshot": true,
        /*
            Сортировать ли скриншоты. Формат: Год-месяц-день
            Например: 2022-02-30

            true - да, сортировать только что созданные скриншоты по папкам
            false - нет, пускай лежат в одной папке как раньше
        */
        "isSortingScreenshots": true,
        /*
            Делает ID игроков цвета *MY CUM*(белого цвета).
            Очень удобно, если у игрока черный ник(одета маска) и на него нужно написать в /rep.

            true - да, я хочу чтобы ID были белого цвета
            false - нет, пускай будет как будет
        */
        "isWhiteID": true,
        /*
            Заменяет старое окно помощи(F1) на более стильное, модное и молодёжное окно!

            true - да, я хочу быть крутым и с новым и обновлённым справочником на F1!!
            false - нет, я хочу гонять со старым окном, т.к. у меня нет вкуса.
        */
        "isCustomF1": true
    },
    /* Всё то, что связано с логотипом GTA RP в верхнем правом углу */
    "serverIcon": {
        /*
            Отображать ли логотип.

            true - да, отображать.
            false - нет, оно не к чему.
        */
        "state": false,
        /*
            Местоположение логотипа на экране по X(право-лево) и Y(верх-вниз).
            Размер иконки(ширина и высота)
        */
        "x": 656.0,
        "y": 28.0,
        "width": 366.0,
        "height": 144.0
    },
    /* Всё, что связано с интерьфесом автомобилья(спидометр и т.д.) */
    "vehicleHud": {
        /*
            Отображать ли табличку, когда посажир садиться в автомобиль.
            P.s. Та самая табличка, которая никому не нужна и на которой написано, куда
            и на какие клавиши нажимать и т.д.

            true - да, пускай будет.
            false - убрать её к **** собачим.
        */
        "isDrawHelpTablet": false,
        /* 
            Убрать ли наледь на окне(При большой скорости она появляется на экране).

            true - да, убрать.
            false - нет, пускай будет.
        */
        "isDisableSnowWindow": true
    }
}

```

---
## Что делать, если я нашёл баг?

Вы можете написать мне в ЛС [_(Контакты есть ниже)_](#Автор).

## Как я могу отблагодарить автора?

<img src="https://www.donationalerts.com/img/header/logo.svg" width="75" height="75">

Донатом на DonationAlerts: https://www.donationalerts.com/r/tim4ukys

## Что он может?

Для этого есть специальный [список](./CHANGE.md). В нём я написал всё, что есть и что планируется добавить.

---
## Автор

[Tim4ukys](https://vk.com/tim4ukys) - Разработчик и простой игрок GTA RP. *Имя кликабельно*</br>
Ник - Antonina_Sobol _(читается как Антонина Соболь, не Собол!!!)_.

### Спасибо

[Шамиль Макаров](https://vk.com/markeazz) - Помощь в тестировании

[Дмитрий Макаров](https://vk.com/molodoy_diman) - Идея

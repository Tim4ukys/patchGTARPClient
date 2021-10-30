[![GitHub license](https://img.shields.io/github/license/Tim4ukys/patchGTARPClient)](https://github.com/Tim4ukys/patchGTARPClient/blob/main/LICENSE)
[![GitHub top language](https://img.shields.io/github/languages/top/Tim4ukys/patchGTARPClient)](https://github.com/Tim4ukys/patchGTARPClient/search?l=c%2B%2B)
[![GitHub tag (latest by date)](https://img.shields.io/github/v/tag/Tim4ukys/patchGTARPClient?label=version)](https://github.com/Tim4ukys/patchGTARPClient/releases/latest)
[![GitHub Release Date](https://img.shields.io/github/release-date/Tim4ukys/patchGTARPClient)](https://github.com/Tim4ukys/patchGTARPClient/releases)

## Patch GTA RP

ASI Plugin который патчит(латает)/улучшает клиент GTA RP.

## Установка

[![Видео](https://img.youtube.com/vi/YC9oGw_tunQ/0.jpg)](https://www.youtube.com/watch?v=YC9oGw_tunQ)

Чтобы установить последнию версию данного плагина, перейдите в [_Release_](https://github.com/Tim4ukys/patchGTARPClient/releases/latest) и скачайте _patchGTARPClientByTim4ukys.rar_. 

В этом архиве будет лежать два файла, Вам нужен именно _!000patchGTARPClientByTim4ukys.ASI_. 

Этот файл нужно будет перенести в корневую папку игры *(эта та папка в которой лежит gta_sa.exe)*. 

## Настройки

Это JSON конфиг с комментариями _"какой параметр и что он делает"_. Не советую копировать его от сюда, ибо при первом запуске(и последующей закрытии) игры он сам появится. Используя json с комментариями есть шанс словить краш(почему? - в душе не ебу).

```json

/*
    Настройки поделены на отдельные структуры. Теперь всё на своих полочках.
*/
{
    /*
        Часы в верхнем правом углу
    */
    "clock": {
        /*
            true - время по МСК(некорректно работает с VPN). 
            false - время с ПК.
        */
        "fixTimeZone": false 
    },
    "samp": {
        /*
            facename шрифта. Параметр полностью бесполезный, если isCustomFont = false
        */
        "fontFaceName": "Segoe UI", 
        /*
            true - во всём клиенте SA-MP будет кастомный шрифт.
            (Тот шрифт который укажите в fontFaceName)
        */
        "isCustomFont": false,
        /*
            true - SA-MP будет делать скриншоты и сохранять их отдельных папках
            (название папки зависит от даты. Например: 30-10-2021)
        */
        "isSortingScreenshots": true,
        /*
            true - Делает ID игроков белыми.
        */ 
        "isWhiteID": true
    },
    "serverIcon": {
        /*
            false - отключает отображение лога сервера
        */
        "state": false,
        /*
            Местоположение лога.
        */
        "x": 656.0,
        "y": 28.0
    },
    "vehicleHud": {
        /*
            false - при посадке в т/с не будет появляться окно с подсказками.
            (на какие клавиши нажимать и вся эта хуйня)
        */
        "isDrawHelpTablet": false
    }
}

```

## Что было сделано и будет сделано?

Для этого я специально сделал [список](./CHANGE.md). 

Вы можете предложить мне идею в ЛС _(Контакты есть ниже)_. Нормальные и адекватные идеи я обязательно добавлю в список, а имя автора в раздел *Спасибо*.

## Авторы

[Tim4ukys](https://vk.com/tim4ukys) - Разработчик и простой игрок GTA RP

### Спасибо

[Шамиль Макаров](https://vk.com/markeazz) - Помощь в тестировании

[Дмитрий Макаров](https://vk.com/molodoy_diman) - Идея
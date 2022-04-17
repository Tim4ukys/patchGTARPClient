from time import sleep
import telebot
import json
import requests
import threading
import tk
from telebot.types import InputMediaPhoto

from db import Database
db = Database('database.db')

bot = telebot.TeleBot(tk.TOKEN_MY_BOT, parse_mode='MARKDOWN');

@bot.message_handler(content_types=['text'])
def get_text_messages(message):
    # print(message.text)
    if message.text == '/check_version':
        d = json.loads(requests.get('https://raw.githubusercontent.com/Tim4ukys/patchGTARPClient/master/update.json').text)
        bot.send_message(message.from_user.id, "*Последняя версия плагина:* _" + d['vers'] + "_")
    elif message.text == "/last_changelog":
        d = json.loads(requests.get('https://raw.githubusercontent.com/Tim4ukys/patchGTARPClient/master/tgbot/changelogs.json').text)
        if len(d['img']) != 0:
            arrImg = d['img']
            arrPhoto = [ InputMediaPhoto(arrImg[0], caption=d['msg']['text'] + "\n\nto be continued", parse_mode=d['msg']['style']) ]
            if len(d['img']) > 1:
                for raw in arrImg[1:]:
                    arrPhoto.append(InputMediaPhoto(raw))
            bot.send_media_group(message.from_user.id, arrPhoto)
        else:
            bot.send_message(message.from_user.id, d['msg']['text'] + "\n\nto be continued", parse_mode=d['msg']['style'])

    elif message.text == "/help":
        bot.send_message(message.from_user.id, 
            "*/check_version* - Узнать актуальную версию плагина на данный момент\n"
            "*/last_changelog* - Узнать, что было сделано в последних обновлениях\n"
            "*/sign_all_changes* - Подписаться/Отписаться на рассылку о выходе обновлений\n"
            "*/vers_info* - Узнать о системе версионировании патча")
    elif message.text == "/sign_all_changes":
        if db.is_active(message.from_user.id):
            db.set_active(message.from_user.id, 0)
            bot.send_message(message.from_user.id, "Вы отписались от рассылки о выходе обновлений.")
        else:
            db.set_active(message.from_user.id, 1)
            bot.send_message(message.from_user.id, "Вы подписались на рассылку о выходе обновлений.")
    elif message.text == "/vers_info":
        bot.send_message(message.from_user.id, 
            requests.get('https://raw.githubusercontent.com/Tim4ukys/patchGTARPClient/master/tgbot/version.txt').text, 
            parse_mode='MarkdownV2')
    elif message.text == "/start":
        if not db.user_exists(message.from_user.id):
            db.add_user(message.from_user.id)

        bot.send_message(message.from_user.id, "qq. Этот бот был создан для того, чтобы держать себя в курсе о выходе обновлений.\n\n"
            "Введите */help* чтобы узнать, что я умею.")
    else:
        bot.send_message(message.from_user.id, "Мой твоя не понимать.\nВведите */help* чтобы узнать, что я умею.")

##########################
    
versPatch = json.loads(requests.get('https://raw.githubusercontent.com/Tim4ukys/patchGTARPClient/master/update.json').text)["vers"]
def checkingUdpdateThread(v_pacth):
    while 1:
        sleep(60)
        newVersPatch = json.loads(requests.get('https://raw.githubusercontent.com/Tim4ukys/patchGTARPClient/master/update.json').text)["vers"]
        if v_pacth != newVersPatch:
            v_pacth = newVersPatch

            arrPhoto = []
            isMedia = False
            d = json.loads(requests.get('https://raw.githubusercontent.com/Tim4ukys/patchGTARPClient/master/tgbot/last_changelog.json').text)
            if len(d['img']) != 0:
                arrImg = d['img']
                arrPhoto = [ InputMediaPhoto(arrImg[0], caption=d['msg']['text'], parse_mode=d['msg']['style']) ]
                if len(d['img']) > 1:
                    for raw in arrImg[1:]:
                        arrPhoto.append(InputMediaPhoto(raw))
                isMedia = True

            users = db.get_active_users()
            for row in users:
                try:
                    if isMedia == True:
                        bot.send_media_group(row[0], arrPhoto)
                    else:
                        bot.send_message(row[0], d['msg']['text'], parse_mode=d['msg']['style'])
                except:
                    db.set_active(row[0], 0)

t = threading.Thread(target=checkingUdpdateThread, args=(versPatch,))
t.start()

##########################

bot.polling(none_stop=True, interval=0)
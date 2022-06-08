from time import sleep
import telebot
import requests
import threading
import tk
import conf
from telebot.types import InputMediaPhoto
from db import Database
from samp_client.client import SampClient
from samp_client.client import ServerInfo

db = Database('database.db')
bot = telebot.TeleBot(tk.TOKEN_MY_BOT, parse_mode='MARKDOWN')
sampInfo = ServerInfo('', 0, 0, '', '', '')
sampServerOffline = True

@bot.message_handler(commands=['check_version'])
def cmd_checkVersion(msg : telebot.types.Message):
    bot.send_message(msg.from_user.id, "*Последняя версия плагина:* _" + requests.get(conf.URL.UPDATE_JSON).json()['vers'] + "_")



@bot.message_handler(commands=['last_changelog'])
def cmd_lastChangelog(msg : telebot.types.Message):
    js = requests.get(conf.URL.CHANGESLOG).json()
    if len(js['img']) != 0:
        arrImg = js['img']
        arrPhoto = [ InputMediaPhoto(arrImg[0], caption=js['msg']['text'] + "\n\nto be continued", parse_mode=js['msg']['style']) ]
        if len(js['img']) > 1:
            for raw in arrImg[1:]:
                arrPhoto.append(InputMediaPhoto(raw))
        bot.send_media_group(msg.from_user.id, arrPhoto)
    else:
        bot.send_message(msg.from_user.id, js['msg']['text'] + "\n\nto be continued", parse_mode=js['msg']['style'])



def thr_online_load():
    sampClient = SampClient('01.gtarp.ru', 7777)
    while True:
        try:
            sampClient.connect()
            gl = globals()
            if not sampClient.is_online():
                # sampServerOffline = True
                gl['sampServerOffline'] = True
                continue
            gl['sampInfo'] = sampClient.get_server_info()
            # sampServerOffline = False
            gl['sampServerOffline'] = False
            sampClient.disconnect()
        except:
            continue
        sleep(30)

tOnlineLoad = threading.Thread(target=thr_online_load)
tOnlineLoad.start()

@bot.message_handler(commands=['online'])
def cmd_online(msg : telebot.types.Message):
    if not sampServerOffline:
        bot.send_message(msg.from_user.id, 
            "*Имя сервера:* {}\n"
            "*Онлайн:* {}/{}".format(sampInfo.hostname, sampInfo.players, sampInfo.max_players))
    else:
        bot.send_message(msg.from_user.id, "*Сервер временно недоступен.* Возможно на нём ведуться тех. работы")



@bot.message_handler(commands=['help'])
def cmd_help(msg : telebot.types.Message):
    bot.send_message(msg.from_user.id, 
        "*/check_version* - Узнать актуальную версию плагина на данный момент\n"
        "*/last_changelog* - Узнать, что было сделано в последних обновлениях\n"
        "*/sign_all_changes* - Подписаться/Отписаться на рассылку о выходе обновлений\n"
        "*/vers_info* - Узнать о системе версионировании патча\n"
        "*/online* - Узнать текущий онлайн на серверах GTA RP")



@bot.message_handler(commands=['sign_all_changes'])
def cmd_signAllChanges(msg : telebot.types.Message):
    if db.is_active(msg.from_user.id):
        db.set_active(msg.from_user.id, 0)
        bot.send_message(msg.from_user.id, "Вы отписались от рассылки о выходе обновлений.")
    else:
        db.set_active(msg.from_user.id, 1)
        bot.send_message(msg.from_user.id, "Вы подписались на рассылку о выходе обновлений.")



@bot.message_handler(commands=['vers_info'])
def cmd_versInfo(msg : telebot.types.Message):
    bot.send_message(msg.from_user.id, requests.get(conf.URL.VERSION_INFO).text, parse_mode='MarkdownV2')



@bot.message_handler(commands=['start'])
def cmd_start(msg : telebot.types.Message):
    if not db.user_exists(msg.from_user.id):
        db.add_user(msg.from_user.id)
    bot.send_message(msg.from_user.id, 
        "qq. Этот бот был создан для того, чтобы держать себя в курсе о выходе обновлений.\n\n"
        "Введите */help* чтобы узнать, что я умею.")



@bot.message_handler(content_types=['text'])
def msgHandler(msg : telebot.types.Message):
    bot.send_message(msg.from_user.id, "Моя твоя не понимать.\nВведите */help* чтобы узнать, что я умею.")

##########################
    
versPatch = requests.get(conf.URL.UPDATE_JSON).json()["vers"]
def checkingUdpdateThread(v_pacth):
    while 1:
        sleep(30)
        newVersPatch = requests.get(conf.URL.UPDATE_JSON).json()["vers"]
        if v_pacth != newVersPatch:
            v_pacth = newVersPatch

            arrPhoto = []
            isMedia = False
            d = requests.get(conf.URL.LAST_UPDATE_CHANGES).json()
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

# bot.polling(none_stop=True, interval=0)
bot.infinity_polling()

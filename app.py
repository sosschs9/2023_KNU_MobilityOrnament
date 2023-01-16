from flask import Flask, render_template
import pymysql
import random

app = Flask(__name__)

db = pymysql.connect(host="localhost", port=3310, user="root", passwd="1234", db="rc_car_db", charset="utf8")
cur = db.cursor()

sql = "SELECT * FROM drive_data"
sql2 = "SELECT * FROM car_data ORDER BY num DESC LIMIT 1"
cur.execute(sql)
drive_data = cur.fetchall()

cur.execute(sql2)
car_data = cur.fetchall()

# 0: id, 1: date, 2: collision, 3: distance, 4: deltha
# 0: id, 1: user, 2: start, 3: end, 4: rental_time

@app.route('/')
def home():
    data_list = []
    data_list2 = []
    collision = 0
    distance = 0
    deltha = 0
    lane = random.randrange(1, 6)
    date = ""

    danger = False

    for data in drive_data:
        if (data[1] != date):
            if (data[2] > 0.0): collision += 1
            if (data[3] > 0.0): distance += 1
            if (data[4] > 0.0): deltha += 1

            data_list2.append([data[1], data[2], data[3], data[4]])
        date = data[1]

    #distance = round(distance / 10)

    data_list.append(collision)
    data_list.append(distance)
    data_list.append(deltha)
    data_list.append(lane)

    # 시간당 얼마 몇시간 대여했는지
    total = collision + distance + deltha
    rental_time = car_data[0][4]

    if ((total / rental_time) > 1):
        danger = True

    return render_template("main.html", data_list=data_list, data_list2=data_list2, car_data=car_data[0], total_data=total, danger=danger)

if __name__ == '__main__':
    app.run()
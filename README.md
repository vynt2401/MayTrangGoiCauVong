
HUONG DAN CAI DAT HE THONG THU THAP DU LIEU (EFR32 -> PYTHON -> MYSQL)


Tai lieu nay huong dan cach thiet lap co so du lieu MySQL (bang Docker) va 
chay script Python de tu dong doc du lieu tu EFR32.

------------------------------------------------------------------------------
PHAN 1: YEU CAU HE THONG (PREREQUISITES)
------------------------------------------------------------------------------
Truoc khi bat dau, may tinh can cai dat:
1. Docker Desktop: De chay MySQL server.
2. Python 3.x: De chay script thu thap du lieu.
3. MySQL Workbench (Khuyen nghi): De xem du lieu.

------------------------------------------------------------------------------
PHAN 2: THIET LAP DATABASE (MYSQL TREN DOCKER)
------------------------------------------------------------------------------

BUOC 1: KHOI CHAY MYSQL CONTAINER
Mo Command Prompt (CMD) hoac PowerShell va chay lenh sau (copy va paste):

docker run --name mysql-dht11 -e MYSQL_ROOT_PASSWORD=root -e MYSQL_DATABASE=sensor_db -p 3306:3306 -d mysql:8.0.41 --default-authentication-plugin=mysql_native_password

>> Thong tin ket noi sau khi chay xong:
   - Host: 127.0.0.1 (localhost)
   - Port: 3306
   - User: root
   - Pass: root
   - DB Name: sensor_db

BUOC 2: TAO BANG DU LIEU (CREATE TABLE)
Mo MySQL Workbench, ket noi vao DB tren, sau do chay doan lenh SQL sau:

USE sensor_db;

CREATE TABLE IF NOT EXISTS dht11_data (
    id INT AUTO_INCREMENT PRIMARY KEY,
    humidity FLOAT NOT NULL,
    temperature FLOAT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

------------------------------------------------------------------------------
PHAN 3: CAI DAT PYTHON LOGGER
------------------------------------------------------------------------------

BUOC 1: CAI THU VIEN
Mo CMD va chay lenh:

pip install pyserial mysql-connector-python

BUOC 2: CAU HINH SCRIPT
Mo file "data_logger_v2.py" bang Notepad hoac VS Code.
Tim dong: SERIAL_PORT = 'COM16'
-> Sua 'COM16' thanh cong COM thuc te ma kit EFR32 dang cam vao.

------------------------------------------------------------------------------
PHAN 4: HUONG DAN VAN HANH (QUAN TRONG)
------------------------------------------------------------------------------

1. NAP CODE CHO KIT:
   Dam bao EFR32 da duoc nap firmware doc DHT11.

2. TAT MOBAXTERM / SERIAL TERMINAL:
   Day la buoc quan trong nhat. Ban phai tat moi ket noi Serial khac 
   dang chiem dung cong COM. Neu khong Python se bao loi "Access is denied".

3. CHAY CHUONG TRINH:
   Tai cua so CMD, go lenh:

   python data_logger_v2.py

4. KIEM TRA KET QUA:
   Man hinh se hien thi:
   --- BAT DAU LOGGER TREN COM... ---
   [SYSTEM] Da ket noi MySQL...
   [RAW]: [OK] Do am: 60.5 % | Nhiet do: 30.2 C
   [DB] Saved: Hum=60.5% | Temp=30.2C


CHUC THANH CONG!

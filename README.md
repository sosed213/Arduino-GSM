# Arduino-GSM
Общением Arduino Mega с GSM модемом.
Проект изначально задуман для дистанционного запуска авто, по СМС, звонку, и/или интернет (еще не придумал, но работу веду над общей основой)


ПРОЕКТ В ПРОЦЕССЕ РАЗРАБОТКИ!!!
В коде присутствует много лишних переменных, функций, обработчиков, но они появились в процессе эксперементов и решении каких то проблем.

Код рабочий.
На данном этапе идет взаимодействие платы SIM900, Arduino Mega, и LCD1602 по шине i2c.  

Реализованны функции: 
* Запрос баланса (Билайн), вывод в Serial и на LCD.
* Получение даты и время, вывод в Serial и на LCD.
* Получение температуры в Омске из интернет источника, вывод в Serial и на LCD.

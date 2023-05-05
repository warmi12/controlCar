# ControlCar

Autorski projekt wykonywany całkowicie samodzielnie, bez pomocy w postaci tutoriali dla ESP czy Raspberry Pi.
Głównym celem jest stworzenie autonomicznego pojazdu.
Do realizacji projektu użyto zwykłego samochodu zdalnie sterowanego, RPI, ESP, PC.
Całość została podzielona na kilka etapów, których połączenie stworzy finalny projekt.
Link do wideo: https://www.youtube.com/watch?v=6WkMlhz5TMg&ab_channel=TheWarmi12

Wydzielone etapy:
1) Sterowanie samochodem za pomocą mikrokontrolera z poziomu PC.
2) Montaż raspberry pi + kamery na samochodzie, wyświetlanie obrazu z kamery na PC.
3) Przetwarzanie obrazu i autonomiczne sterowanie samochodem.

# Etap 1 
Celem pierwszego etapu jest upewnienie się, że przyjęta koncepcja sterowania działa i na jej podstawie można oprzeć dalszy rozwój projektu.
Pomysł sterowania polega na wlutowaniu w odpowiednich miejscach pilota przewodów, które zostaną podłączone do wyprowadzeń mikrokontrolera ESP32.
Zadaniem ESP jest wysterowanie odpowiednich pinów układu znajdującego się w pilocie, co umożliwi sterowanie.
Zaletą takiego rozwiązania jest mniejsza waga pojazdu oraz mniejsze zużycie energii. 
Bezprzewodowa komunikacja pomiędzy ESP a PC zapewniona jest za pomocą protokołu MQTT.
Na komputerze uruchomiony jest skrypt, który nasłuchuje input'u z klawiatury oraz wysyła dane do brokera MQTT.
Po odebraniu danych i ich obrobieniu wysterowywany jest odpowiedni pin ESP.
Komunikacja przedstawiona jest na poniższym schemacie:

<img
  src="/imgs/img0.png"
  alt="Alt text"
  title="Optional title"
  style="display: inline-block; margin: 0 auto; max-width: 300px">

##

<img
  src="/imgs/img1.jpg"
  alt="Alt text"
  title="Optional title"
  style="display: inline-block; margin: 0 auto; max-width: 300px">


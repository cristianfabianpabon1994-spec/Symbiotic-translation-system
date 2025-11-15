# elimin yaw, vector_actual es de 4 elementos

from http.server import BaseHTTPRequestHandler, HTTPServer
import urllib.parse
import threading
import time
import keyboard
import csv
import os
import glob
import numpy as np

grabando = False
datos_grabados = []
inicio_grabacion = 0
duracion_grabacion = 10  # segundos
letras_registradas = {}

def guardar_datos(letra, datos):
    nombre_archivo = f"datos_{letra}_{int(time.time())}.csv"
    with open(nombre_archivo, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['timestamp', 'pitch0', 'roll0', 'pitch1', 'roll1'])
        writer.writerows(datos)
    print(f"✅ Datos guardados en {nombre_archivo}")
    cargar_letras()

def cargar_letras():
    global letras_registradas
    letras_registradas = {}
    for archivo in glob.glob("datos_*.csv"):
        letra = archivo.split("_")[1].upper()
        datos = np.genfromtxt(archivo, delimiter=',', skip_header=1)
        if datos.ndim == 1:
            continue
        promedio = np.mean(datos[:, 1:], axis=0)
        letras_registradas[letra] = promedio
    print(f"🔤 Letras registradas: {list(letras_registradas.keys())}")

def reconocer_letra(vector_actual):
    if not letras_registradas:
        return None
    min_dist = float('inf')
    letra_detectada = None
    for letra, vector_ref in letras_registradas.items():
        dist = np.linalg.norm(vector_actual - vector_ref)
        if dist < min_dist:
            min_dist = dist
            letra_detectada = letra
    if min_dist < 10:
        return letra_detectada
    return None

class MiServidor(BaseHTTPRequestHandler):
    def do_GET(self):
        global grabando, datos_grabados, inicio_grabacion
        ruta = urllib.parse.urlparse(self.path)

        if ruta.path == "/datos":
            parametros = urllib.parse.parse_qs(ruta.query)

            # Solo pitch y roll
            pitch0 = float(parametros.get('pitch_IP', ['0'])[0])
            roll0  = float(parametros.get('roll_IP',  ['0'])[0])
            pitch1 = float(parametros.get('pitch_II', ['0'])[0])
            roll1  = float(parametros.get('roll_II',  ['0'])[0])

            vector_actual = np.array([pitch0, roll0, pitch1, roll1])

            print(f"Sensor A -> Pitch: {pitch0} | Roll: {roll0}")
            print(f"Sensor B -> Pitch: {pitch1} | Roll: {roll1}")

            if grabando:
                tiempo_actual = time.time()
                if tiempo_actual - inicio_grabacion <= duracion_grabacion:
                    datos_grabados.append([
                        tiempo_actual,
                        pitch0, roll0,
                        pitch1, roll1
                    ])
                else:
                    grabando = False
                    print("\n🛑 Grabación finalizada.")
                    letra = input("➡️ Ingresa la letra que corresponde a esta grabación: ").strip().upper()
                    guardar_datos(letra, datos_grabados)
                    datos_grabados.clear()
            else:
                letra = reconocer_letra(vector_actual)
                if letra:
                    print(f"🔍 Letra detectada: {letra}")

            self.send_response(200)
            self.send_header("Content-type", "text/plain")
            self.end_headers()
            self.wfile.write(b"OK\n")
        else:
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b"Ruta no encontrada\n")

def escuchar_teclas():
    global grabando, inicio_grabacion
    print("⌨️ Presiona 'r' para grabar una nueva letra (10 segundos)...")
    while True:
        keyboard.wait('r')
        if not grabando:
            grabando = True
            inicio_grabacion = time.time()
            print("\n⏺️ Iniciando grabación de datos...")

if __name__ == "__main__":
    IP_PC = "192.168.137.1"
    PUERTO = 80

    cargar_letras()

    hilo_teclado = threading.Thread(target=escuchar_teclas, daemon=True)
    hilo_teclado.start()

    try:
        servidor = HTTPServer((IP_PC, PUERTO), MiServidor)
        print(f"🚀 Servidor iniciado en http://{IP_PC}:{PUERTO}")
        servidor.serve_forever()
    except OSError as e:
        print(f"❌ Error al iniciar servidor: {e}")

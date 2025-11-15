from http.server import BaseHTTPRequestHandler, HTTPServer
import urllib.parse
import numpy as np
import glob

letras_registradas = {}

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
    print(f"📁 Letras cargadas: {list(letras_registradas.keys())}")

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
    if min_dist < 50:
        return letra_detectada
    return None

class MiServidor(BaseHTTPRequestHandler):
    def do_GET(self):
        ruta = urllib.parse.urlparse(self.path)
        if ruta.path == "/datos":
            parametros = urllib.parse.parse_qs(ruta.query)

            try:
                pitch0 = float(parametros.get('pitch_IP', ['0'])[0])
                roll0  = float(parametros.get('roll_IP',  ['0'])[0])
                pitch1 = float(parametros.get('pitch_II', ['0'])[0])
                roll1  = float(parametros.get('roll_II',  ['0'])[0])
            except ValueError:
                self.send_response(400)
                self.end_headers()
                self.wfile.write(b"Error en los datos\n")
                return

            vector_actual = np.array([pitch0, roll0, pitch1, roll1])

            letra = reconocer_letra(vector_actual)
            if letra:
                print(f"🧠 Letra detectada: {letra}")
            else:
                print("🤔 Sin coincidencias claras.")

            self.send_response(200)
            self.send_header("Content-type", "text/plain")
            self.end_headers()
            self.wfile.write(f"Letra detectada: {letra or 'Ninguna'}\n".encode())

        else:
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b"Ruta no encontrada\n")

if __name__ == "__main__":
    IP_PC = "192.168.137.1"
    PUERTO = 80

    cargar_letras()

    try:
        servidor = HTTPServer((IP_PC, PUERTO), MiServidor)
        print(f"📡 Servidor de reconocimiento iniciado en http://{IP_PC}:{PUERTO}")
        servidor.serve_forever()
    except OSError as e:
        print(f"❌ Error al iniciar servidor: {e}")

from flask import Flask, request, jsonify, render_template
from flask_socketio import SocketIO, emit

app = Flask(__name__)
socketio = SocketIO(app)

ppm_atual = 0


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/update", methods=["GET"])
def update():
    global ppm_atual

    ppm = request.args.get("ppm", type=int)

    if ppm is None:
        return jsonify(error="ppm ausente ou inválido"), 400

    ppm_atual = int(ppm)

    socketio.emit("ppm_update", {"ppm": ppm_atual})

    return jsonify(ok=True, ppm=ppm_atual)


@socketio.on("connect")
def on_connect():
    emit("ppm_update", {"ppm": ppm_atual})


if __name__ == "__main__":
    socketio.run(app, host="0.0.0.0", port=5000, debug=True)

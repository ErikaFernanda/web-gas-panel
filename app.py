from flask import Flask, request, jsonify, render_template
import json

app = Flask(__name__)



@app.route("/")
def index():
    return render_template("index.html")


@app.route("/read")
def read():

    with open("data.json", "r", encoding="utf-8") as f:
        dados = json.load(f)
    return jsonify(ok=True, ppm=dados["value"])


@app.route("/update", methods=["GET"])
def update():
    ppm = request.args.get("ppm", type=int)
    with open("data.json", "r", encoding="utf-8") as f:
        dados = json.load(f) 
    dados["value"] = ppm
    with open("data.json", "w", encoding="utf-8") as f:
        json.dump(dados, f, ensure_ascii=False, indent=2)

    return jsonify(ok=True, ppm=ppm)


if __name__ == "__main__":
    app.run()

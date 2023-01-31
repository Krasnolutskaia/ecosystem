from flask import Flask, jsonify, render_template
from flask import request
import os
from forms import AngleForm

app = Flask(__name__)

status = {'rotate': 0, 'angle': 0, 'auto_rotate': 0, 'auto_light': 0, 'temp': -1.0, 'hum_air': -1.0, 'hum_soil': 0,
          'auto_water': 0, 'water': 0, 'water_volume': 0,
          'lights': [{'n': 1, 'val': 0}, {'n': 2, 'val': 0}, {'n': 3, 'val': 0}, {'n': 4, 'val': 0}]}


@app.route('/')
def index():
    angle = request.args.get('angle', '')
    if angle:
        status['rotate'] = 1
    return render_template('index.html', temp=status['temp'], hum_air=status['hum_air'], hum_soil=status['hum_soil'],
                           lights=status['lights'])


@app.route('/water', methods=["POST"])
def water():
    if request.json['water']:
        status['water'] = 1

    else:
        status['water'] = 0
    return 'ok'


@app.route('/water_volume', methods=["POST"])
def water_volume():
    pass


@app.route('/light', methods=["POST"])
def light():
    if request.json['water']:
        status['water'] = 1
    else:
        status['water'] = 0
    return 'ok'


@app.route('/from_greenhouse', methods=["POST"])
def from_greenhouse():
    if request.method == "POST":
        hum_air = request.form.get('hum_air')
        temp = request.form.get('temp')
        status['hum_soil'] = request.form.get('hum_soil')

        for i in range(4):
            status['lights'][i]['val'] = request.form.get(f'light_val{i}')

        try:
            status['hum_air'] = float(hum_air)
            status['temp'] = float(temp)
        except:
            status['hum_air'] = 0
            status['temp'] = 0
    print(status)
    return 'ok'


@app.route('/to_greenhouse')
def to_greenhouse():
    return jsonify(status)


if __name__ == '__main__':
    port = int(os.environ.get("PORT", 80))
    app.run(debug=True, host='192.168.101.229', port=port)

import time
import urllib2
import random

_RGB = [0, 1, 2]

def changeLights(rgb):
    url = 'http://192.168.1.148/' + str(int(rgb[0])) + '/' + str(int(rgb[1])) + '/' + str(int(rgb[2]))
    req = urllib2.Request(url, None)
    try:
        response = urllib2.urlopen(req, None, .1)
    except urllib2.URLError as e:
        if str(e.reason) == 'timed out':
            return
        print '\t' + str(e.reason)
        if str(e.reason) == '[Errno 64] Host is down':
            time.sleep(1)
    except urllib2.socket.timeout as e:
        return


def printVal(rgb):
    print str(rgb[0]) + '\t' + str(rgb[1]) + '\t' + str(rgb[2])




baseIndex = 0
toColor = [0, 0, 1]

while True:

    _MODES = ['schizm', 'basis', 'white']
    mode = _MODES[1]

    # schizm
    if mode == _MODES[0]:
        changeLights([255,0,0])
        changeLights([0,255,0])
        changeLights([0,0,255])


    # rotate through basis
    elif mode == _MODES[1]:

        _BASE_MAX = 255
        _BASE_RANDOMNESS = 0
        # setting constant transition power to 0 will ignore that calculation altogether
        _CONSTANT_TRANSITION_POWER = 255

        basis = [
            [0, 0, 1],
            [0, 1, 1],
            [0, 1, 0],
            [1, 1, 0],
            [1, 0, 0],
            [1, 0, 1],
        ]

        atColor = list(toColor)
        toColor = list(basis[baseIndex])
        baseIndex = (baseIndex + 1) % len(basis)

        # randomize rotation
        for rgb in _RGB:
            toColor[rgb] = toColor[rgb] * (_BASE_MAX - _BASE_RANDOMNESS) + random.randint(0, _BASE_RANDOMNESS);

        print 'From: ' + str(atColor) + '\tTo: ' + str(toColor)

        steps = 200
        for step in range(0, steps):
            thisColor = [0, 0, 0]
            for rgb in _RGB:
                thisColor[rgb] = int(atColor[rgb] + (toColor[rgb] - atColor[rgb]) * step / float(steps))
            # constant power through transition
            if _CONSTANT_TRANSITION_POWER and sum(thisColor):
                oldsum = float(sum(thisColor))
                for rgb in _RGB:
                    thisColor[rgb] = int(thisColor[rgb] * _CONSTANT_TRANSITION_POWER / oldsum)
            printVal(thisColor)

            changeLights(thisColor);


    # whiteness
    elif mode == _MODES[2]:

        _BRIGHTNESS = 1

        changeLights([_BRIGHTNESS, _BRIGHTNESS, _BRIGHTNESS])


    # blackness
    else:

        changeLights([0, 0, 0])


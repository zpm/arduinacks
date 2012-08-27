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
toColor = [0, 0, 0]

while True:

    mode = 'basis'

    # schizm
    if mode == 'schizm':

        _SPEED = .2
        _ROTATE = [0,255,255]

        changeLights(_ROTATE)
        time.sleep(_SPEED)
        changeLights([_ROTATE[1], _ROTATE[2], _ROTATE[0]])
        time.sleep(_SPEED)
        changeLights([_ROTATE[2], _ROTATE[0], _ROTATE[1]])
        time.sleep(_SPEED)


    # rotate through basis
    elif mode == 'basis':

        _BASE_MAX = 255
        _BASE_RANDOMNESS = 100
        # setting constant transition power to 0 will ignore that calculation altogether
        _CONSTANT_TRANSITION_POWER = 255

        _TRANSITION_STEPS = 100

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

        for step in range(0, _TRANSITION_STEPS):
            thisColor = [0, 0, 0]
            for rgb in _RGB:
                thisColor[rgb] = int(atColor[rgb] + (toColor[rgb] - atColor[rgb]) * step / float(_TRANSITION_STEPS))
            # constant power through transition
            if _CONSTANT_TRANSITION_POWER and sum(thisColor):
                oldsum = float(sum(thisColor))
                for rgb in _RGB:
                    thisColor[rgb] = int(thisColor[rgb] * _CONSTANT_TRANSITION_POWER / oldsum)
            printVal(thisColor)

            changeLights(thisColor);


    # whiteness
    elif mode == 'blinder':

        _BRIGHTNESS = 255

        changeLights([_BRIGHTNESS, _BRIGHTNESS, _BRIGHTNESS])


    # blackness
    else:

        changeLights([0, 0, 0])


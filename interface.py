from PyQt5 import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from pyqtgraph import PlotWidget
import sys
from random import *
#====#
temps=[x for x in range(100)]
altitude=[10 for x in range(100)]

#====#

#====#

def testgraph():
    global temps,altitude
    temps=temps[1:]
    altitude=altitude[1:]
    temps+=[temps[-1]+1]
    altitude+=[altitude[-1]+randint(0,10)]
    Altitude_data.setText(str(altitude[-1]))
    graph.clear()
    graph.plot(temps,altitude)


#====#

interface=QApplication(sys.argv)

central=QWidget()
layout=QGridLayout()
central.setLayout(layout)

time=QTimer()
time.setInterval(1000)
time.timeout.connect(testgraph)
time.start()

wl_graph=QWidget()
wl_graph.setGeometry(1000,100,700,700)

l_graph=QHBoxLayout()
wl_graph.setLayout(l_graph)

graph=PlotWidget()
l_graph.addWidget(graph)

#graph.plot(temps,altitude)

layout.addWidget(wl_graph,0,0)


l_data=QGridLayout()
layout.addLayout(l_data,0,1)



## Gps
l_Gps=QHBoxLayout()
Gps=QLabel(text="GPS:")
Gps.setAlignment((Qt.AlignRight)|(Qt.AlignVCenter))
l_Gps.addWidget(Gps)

Gps_data=QLabel(text="Latitude; Longitude")
Gps_data.setAlignment((Qt.AlignLeft)|(Qt.AlignVCenter))
l_Gps.addWidget(Gps_data)
l_data.addLayout(l_Gps,0,0)

##


## Altitude
l_Altitude=QHBoxLayout()
Altitude=QLabel(text="Altitude:")
Altitude.setAlignment((Qt.AlignRight)|(Qt.AlignVCenter))
l_Altitude.addWidget(Altitude)

Altitude_data=QLabel(text="Altitude")
Altitude_data.setAlignment((Qt.AlignLeft)|(Qt.AlignVCenter))
l_Altitude.addWidget(Altitude_data)
l_data.addLayout(l_Altitude,1,0)


##


## Angle
l_Angle=QHBoxLayout()
Angle=QLabel(text="Angle:")
Angle.setAlignment((Qt.AlignRight)|(Qt.AlignVCenter))
l_Angle.addWidget(Angle)

Angle_data=QLabel(text="accelerometer[3]")
Angle_data.setAlignment((Qt.AlignLeft)|(Qt.AlignVCenter))
l_Angle.addWidget(Angle_data)
l_data.addLayout(l_Angle,2,0)


##



## vitesse
l_vitesse=QHBoxLayout()
vitesse=QLabel(text="vitesse:")
vitesse.setAlignment((Qt.AlignRight)|(Qt.AlignVCenter))
l_vitesse.addWidget(vitesse)

vitesse_data=QLabel(text="velocity[3]")
vitesse_data.setAlignment((Qt.AlignLeft)|(Qt.AlignVCenter))
l_vitesse.addWidget(vitesse_data)
l_data.addLayout(l_vitesse,3,0)


##



## Pression
l_Pression=QHBoxLayout()
Pression=QLabel(text="Pression:")
Pression.setAlignment((Qt.AlignRight)|(Qt.AlignVCenter))
l_Pression.addWidget(Pression)

Pression_data=QLabel(text="pressure")
Pression_data.setAlignment((Qt.AlignLeft)|(Qt.AlignVCenter))
l_Pression.addWidget(Pression_data)
l_data.addLayout(l_Pression,4,0)


##











central.show()


interface.exec()

import os
from NodeGraphQt import NodeGraph, BaseNode, BackdropNode, setup_context_menu
from NodeGraphQt import QtWidgets, QtCore, PropertiesBinWidget, NodeTreeWidget
import nodes

GRAPH = None
PROP = None
def init_graph():
    global GRAPH, PROP

    GRAPH = NodeGraph()
    setup_context_menu(GRAPH)
    GRAPH.register_node(nodes.Model)

    PROP = PropertiesBinWidget(node_graph=GRAPH)
    PROP.setWindowFlags(QtCore.Qt.Tool)
    GRAPH.node_double_clicked.connect(show_props)

    GRAPH.widget.resize(1000, 800)
    GRAPH.widget.show()

def show_props(node):
    if not PROP.isVisible():
        PROP.show()

if __name__ == '__main__':
    app = QtWidgets.QApplication([])
    init_graph()
    app.exec_()
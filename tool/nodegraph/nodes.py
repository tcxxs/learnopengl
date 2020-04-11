from NodeGraphQt import BaseNode

class Model(BaseNode):
    __identifier__ = 'graph'
    NODE_NAME = 'Model'

    def __init__(self):
        super(Model, self).__init__()

        # create node inputs.
        self.add_input('in A')
        self.add_input('in B')

        # create node outputs.
        self.add_output('out A')
        self.add_output('out B')
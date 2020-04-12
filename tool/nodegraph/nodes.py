import yaml
from NodeGraphQt import BaseNode

class Shader(BaseNode):
    SHADERS = []
    CONFIG = {}

    @staticmethod
    def load_shaders(fname):
        with open(fname, 'rb') as f:
            confs = yaml.load(f)
        
        for name, config in confs.items():
            cls = type(name, (Shader,), {})
            cls.__identifier__ = 'shaders'
            cls.NODE_NAME = name
            cls.CONFIG = config
            Shader.SHADERS.append(cls)

    def __init__(self):
        super(Shader, self).__init__()

        for name, vtype in self.__class__.CONFIG['input'].items():
            if vtype == 'bool':
                self.add_checkbox(name, '', name)
            else:
                self.add_input(name)
        for name, vtype in self.__class__.CONFIG['output'].items():
            self.add_output(name)

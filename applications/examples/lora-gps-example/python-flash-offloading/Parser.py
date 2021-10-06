import logging
from base64 import b64decode
from bitstring import ConstBitStream

logger = logging.getLogger(__name__)

class Parser:
    def parse(self, payload):
        s = ConstBitStream(hex=payload)
        parsed_data = {}
        while s.pos < s.len:
            msg_type = s.read("uint:8")
            d = {
                25: self.parse_octa_status,
                36: self.parse_octa_id,
                21: self.parse_imsi,
                34: self.parse_message_counter_extra_large,
                26: self.parse_rod_id,
                27: self.parse_rod_impedance,
                28: self.parse_rod_impedance_phase,
                29: self.parse_rod_error,
                30: self.parse_rod_voltage_a,
                31: self.parse_rod_voltage_b,
                32: self.parse_rod_voltage_c,
                33: self.parse_rod_temperature,
                4: self.parse_temp,
                5: self.parse_hum,
                24: self.parse_battery,
                255: self.parse_ignore
            }[msg_type](s)
            parsed_data[self.msg_type_to_string(msg_type)] = d
        
        return parsed_data

    @staticmethod
    def msg_type_to_string(msg_type):
        if msg_type == 25: return "octa status"
        if msg_type == 36: return "octa id"
        if msg_type == 21: return "imsi"
        if msg_type == 34: return "msg counter"
        if msg_type == 26: return "rod id"
        if msg_type == 27: return "rod impedance"
        if msg_type == 28: return "rod impedance phase"
        if msg_type == 29: return "rod error"
        if msg_type == 30: return "rod voltage a"
        if msg_type == 31: return "rod voltage b"
        if msg_type == 32: return "rod voltage c"
        if msg_type == 33: return "rod temperature"
        if msg_type == 4: return "box temperature"
        if msg_type == 5: return "box humidity"
        if msg_type == 24: return "battery"
        else: return
        return "unknown"
    
    @staticmethod
    def parse_octa_status(s):
        return s.read('uintle:32')
    
    @staticmethod
    def parse_octa_id(s):
        return s.read("uintle:64")

    @staticmethod
    def parse_imsi(s):
        return s.read("uintle:64")

    @staticmethod
    def parse_message_counter_extra_large(s):
        return s.read("intle:32")

    @staticmethod
    def parse_rod_id(s):
        return s.read("intle:32")

    @staticmethod
    def parse_rod_impedance(s):
        return s.read("intle:32")

    @staticmethod
    def parse_rod_impedance_phase(s):
        return s.read("intle:16")

    @staticmethod
    def parse_rod_error(s):
        return s.read("uintle:8")

    @staticmethod
    def parse_rod_voltage_a(s):
        return s.read("intle:32")

    @staticmethod
    def parse_rod_voltage_b(s):
        return s.read("intle:32")

    @staticmethod
    def parse_rod_voltage_c(s):
        return s.read("intle:32")

    @staticmethod
    def parse_rod_temperature(s):
        return s.read("intle:16")

    @staticmethod
    def parse_temp(s):
        return s.read("floatle:32")

    @staticmethod
    def parse_hum(s):
        return s.read("floatle:32")

    @staticmethod
    def parse_battery(s):
        return s.read("uintle:16")
    
    @staticmethod
    def parse_ignore(s):
        return
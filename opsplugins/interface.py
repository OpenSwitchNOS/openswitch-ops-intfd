from opsvalidator.base import *
from opsvalidator import error
from opsvalidator.error import ValidationError


class InterfaceValidator(BaseValidator):
    resource = "interface"
    def validate_deletion(self, validation_args):
        details = "Physical interfaces cannot be deleted"
        raise ValidationError(error.VERIFICATION_FAILED, details)

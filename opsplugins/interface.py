from opsvalidator.base import BaseValidator
from opsvalidator import error
from opsvalidator.error import ValidationError
from opsrest.utils.utils import get_column_data_from_row


class InterfaceValidator(BaseValidator):
    resource = "interface"
    def validate_deletion(self, validation_args):
        intf_row = validation_args.resource_row
        intf_type = get_column_data_from_row(intf_row, "type")
        if intf_type == 'system':
            details = "Physical interfaces cannot be deleted"
            raise ValidationError(error.VERIFICATION_FAILED, details)

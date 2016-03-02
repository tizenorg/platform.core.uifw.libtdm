#include "tdm-test.h"

int
tdm_test_output_set(tdm_test_data *data)
{
	output_arg *output = NULL;
	tdm_error ret;

	LIST_FOR_EACH_ENTRY(output, &data->args.output_list, link) {
		const tdm_output_mode *modes;
		prop_arg *prop = NULL;
		int i, count;

		ret = tdm_output_get_available_modes(output, &modes, &count);
		return_val_if_fail(ret == TDM_ERROR_NONE, 0);
		for (i = 0; i < count; i++) {
			if (!strncmp(modes[i].name, output->mode, TDM_NAME_LEN)) {
				tdm_output_set_mode(output, &modes[i]);
				break;
			}
		}

		LIST_FOR_EACH_ENTRY(prop, &output->prop_list, link) {
			const tdm_prop *props;
			tdm_value value;

			ret = tdm_output_get_available_properties(output, &props, &count);
			return_val_if_fail(ret == TDM_ERROR_NONE, 0);
			for (i = 0; i < count; i++) {
				if (!strncmp(props[i].name, prop->name, TDM_NAME_LEN)) {
					value.s32 = prop->value;
					tdm_output_set_property(output, props[i].id, value);
					break;
				}
			}
		}
	}

	return 1;
}

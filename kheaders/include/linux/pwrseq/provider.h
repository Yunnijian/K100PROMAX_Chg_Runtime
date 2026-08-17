/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __POWER_SEQUENCING_PROVIDER_H__
#define __POWER_SEQUENCING_PROVIDER_H__

struct device;
struct module;
struct pwrseq_device;

typedef int (*pwrseq_power_state_func)(struct pwrseq_device *);
typedef int (*pwrseq_match_func)(struct pwrseq_device *, struct device *);


struct pwrseq_unit_data {
	const char *name;
	const struct pwrseq_unit_data **deps;
	pwrseq_power_state_func enable;
	pwrseq_power_state_func disable;
};


struct pwrseq_target_data {
	const char *name;
	const struct pwrseq_unit_data *unit;
	pwrseq_power_state_func post_enable;
};


struct pwrseq_config {
	struct device *parent;
	struct module *owner;
	void *drvdata;
	pwrseq_match_func match;
	const struct pwrseq_target_data **targets;
};

struct pwrseq_device *
pwrseq_device_register(const struct pwrseq_config *config);
void pwrseq_device_unregister(struct pwrseq_device *pwrseq);
struct pwrseq_device *
devm_pwrseq_device_register(struct device *dev,
			    const struct pwrseq_config *config);

void *pwrseq_device_get_drvdata(struct pwrseq_device *pwrseq);

#endif 

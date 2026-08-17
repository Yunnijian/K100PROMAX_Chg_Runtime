/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_PWM_H
#define __LINUX_PWM_H

#include <linux/device.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/android_kabi.h>

MODULE_IMPORT_NS(PWM);

struct pwm_chip;


enum pwm_polarity {
	PWM_POLARITY_NORMAL,
	PWM_POLARITY_INVERSED,
};


struct pwm_args {
	u64 period;
	enum pwm_polarity polarity;
};

enum {
	PWMF_REQUESTED = 0,
	PWMF_EXPORTED = 1,
};


struct pwm_state {
	u64 period;
	u64 duty_cycle;
	enum pwm_polarity polarity;
	bool enabled;
	bool usage_power;
};


struct pwm_device {
	const char *label;
	unsigned long flags;
	unsigned int hwpwm;
	struct pwm_chip *chip;

	struct pwm_args args;
	struct pwm_state state;
	struct pwm_state last;

	ANDROID_KABI_RESERVE(1);
};


static inline void pwm_get_state(const struct pwm_device *pwm,
				 struct pwm_state *state)
{
	*state = pwm->state;
}

static inline bool pwm_is_enabled(const struct pwm_device *pwm)
{
	struct pwm_state state;

	pwm_get_state(pwm, &state);

	return state.enabled;
}

static inline u64 pwm_get_period(const struct pwm_device *pwm)
{
	struct pwm_state state;

	pwm_get_state(pwm, &state);

	return state.period;
}

static inline u64 pwm_get_duty_cycle(const struct pwm_device *pwm)
{
	struct pwm_state state;

	pwm_get_state(pwm, &state);

	return state.duty_cycle;
}

static inline enum pwm_polarity pwm_get_polarity(const struct pwm_device *pwm)
{
	struct pwm_state state;

	pwm_get_state(pwm, &state);

	return state.polarity;
}

static inline void pwm_get_args(const struct pwm_device *pwm,
				struct pwm_args *args)
{
	*args = pwm->args;
}


static inline void pwm_init_state(const struct pwm_device *pwm,
				  struct pwm_state *state)
{
	struct pwm_args args;

	
	pwm_get_state(pwm, state);

	
	pwm_get_args(pwm, &args);

	state->period = args.period;
	state->polarity = args.polarity;
	state->duty_cycle = 0;
	state->usage_power = false;
}


static inline unsigned int
pwm_get_relative_duty_cycle(const struct pwm_state *state, unsigned int scale)
{
	if (!state->period)
		return 0;

	return DIV_ROUND_CLOSEST_ULL((u64)state->duty_cycle * scale,
				     state->period);
}


static inline int
pwm_set_relative_duty_cycle(struct pwm_state *state, unsigned int duty_cycle,
			    unsigned int scale)
{
	if (!scale || duty_cycle > scale)
		return -EINVAL;

	state->duty_cycle = DIV_ROUND_CLOSEST_ULL((u64)duty_cycle *
						  state->period,
						  scale);

	return 0;
}


struct pwm_capture {
	unsigned int period;
	unsigned int duty_cycle;
};


struct pwm_ops {
	int (*request)(struct pwm_chip *chip, struct pwm_device *pwm);
	void (*free)(struct pwm_chip *chip, struct pwm_device *pwm);
	int (*capture)(struct pwm_chip *chip, struct pwm_device *pwm,
		       struct pwm_capture *result, unsigned long timeout);
	int (*apply)(struct pwm_chip *chip, struct pwm_device *pwm,
		     const struct pwm_state *state);
	int (*get_state)(struct pwm_chip *chip, struct pwm_device *pwm,
			 struct pwm_state *state);
	ANDROID_KABI_RESERVE(1);
};


struct pwm_chip {
	struct device dev;
	const struct pwm_ops *ops;
	struct module *owner;
	unsigned int id;
	unsigned int npwm;

	struct pwm_device * (*of_xlate)(struct pwm_chip *chip,
					const struct of_phandle_args *args);
	bool atomic;

	
	bool uses_pwmchip_alloc;
	ANDROID_KABI_RESERVE(1);
	struct pwm_device pwms[] __counted_by(npwm);
};

static inline struct device *pwmchip_parent(const struct pwm_chip *chip)
{
	return chip->dev.parent;
}

static inline void *pwmchip_get_drvdata(struct pwm_chip *chip)
{
	return dev_get_drvdata(&chip->dev);
}

static inline void pwmchip_set_drvdata(struct pwm_chip *chip, void *data)
{
	dev_set_drvdata(&chip->dev, data);
}

#if IS_ENABLED(CONFIG_PWM)

int pwm_apply_might_sleep(struct pwm_device *pwm, const struct pwm_state *state);
int pwm_apply_atomic(struct pwm_device *pwm, const struct pwm_state *state);
int pwm_adjust_config(struct pwm_device *pwm);


static inline int pwm_config(struct pwm_device *pwm, int duty_ns,
			     int period_ns)
{
	struct pwm_state state;

	if (!pwm)
		return -EINVAL;

	if (duty_ns < 0 || period_ns < 0)
		return -EINVAL;

	pwm_get_state(pwm, &state);
	if (state.duty_cycle == duty_ns && state.period == period_ns)
		return 0;

	state.duty_cycle = duty_ns;
	state.period = period_ns;
	return pwm_apply_might_sleep(pwm, &state);
}


static inline int pwm_enable(struct pwm_device *pwm)
{
	struct pwm_state state;

	if (!pwm)
		return -EINVAL;

	pwm_get_state(pwm, &state);
	if (state.enabled)
		return 0;

	state.enabled = true;
	return pwm_apply_might_sleep(pwm, &state);
}


static inline void pwm_disable(struct pwm_device *pwm)
{
	struct pwm_state state;

	if (!pwm)
		return;

	pwm_get_state(pwm, &state);
	if (!state.enabled)
		return;

	state.enabled = false;
	pwm_apply_might_sleep(pwm, &state);
}


static inline bool pwm_might_sleep(struct pwm_device *pwm)
{
	return !pwm->chip->atomic;
}


void pwmchip_put(struct pwm_chip *chip);
struct pwm_chip *pwmchip_alloc(struct device *parent, unsigned int npwm, size_t sizeof_priv);
struct pwm_chip *devm_pwmchip_alloc(struct device *parent, unsigned int npwm, size_t sizeof_priv);

int __pwmchip_add(struct pwm_chip *chip, struct module *owner);
#define pwmchip_add(chip) __pwmchip_add(chip, THIS_MODULE)
void pwmchip_remove(struct pwm_chip *chip);

int __devm_pwmchip_add(struct device *dev, struct pwm_chip *chip, struct module *owner);
#define devm_pwmchip_add(dev, chip) __devm_pwmchip_add(dev, chip, THIS_MODULE)

struct pwm_device *of_pwm_xlate_with_flags(struct pwm_chip *chip,
		const struct of_phandle_args *args);
struct pwm_device *of_pwm_single_xlate(struct pwm_chip *chip,
				       const struct of_phandle_args *args);

struct pwm_device *pwm_get(struct device *dev, const char *con_id);
void pwm_put(struct pwm_device *pwm);

struct pwm_device *devm_pwm_get(struct device *dev, const char *con_id);
struct pwm_device *devm_fwnode_pwm_get(struct device *dev,
				       struct fwnode_handle *fwnode,
				       const char *con_id);
#else
static inline bool pwm_might_sleep(struct pwm_device *pwm)
{
	return true;
}

static inline int pwm_apply_might_sleep(struct pwm_device *pwm,
					const struct pwm_state *state)
{
	might_sleep();
	return -EOPNOTSUPP;
}

static inline int pwm_apply_atomic(struct pwm_device *pwm,
				   const struct pwm_state *state)
{
	return -EOPNOTSUPP;
}

static inline int pwm_adjust_config(struct pwm_device *pwm)
{
	return -EOPNOTSUPP;
}

static inline int pwm_config(struct pwm_device *pwm, int duty_ns,
			     int period_ns)
{
	might_sleep();
	return -EINVAL;
}

static inline int pwm_enable(struct pwm_device *pwm)
{
	might_sleep();
	return -EINVAL;
}

static inline void pwm_disable(struct pwm_device *pwm)
{
	might_sleep();
}

static inline void pwmchip_put(struct pwm_chip *chip)
{
}

static inline struct pwm_chip *pwmchip_alloc(struct device *parent,
					     unsigned int npwm,
					     size_t sizeof_priv)
{
	return ERR_PTR(-EINVAL);
}

static inline struct pwm_chip *devm_pwmchip_alloc(struct device *parent,
						  unsigned int npwm,
						  size_t sizeof_priv)
{
	return pwmchip_alloc(parent, npwm, sizeof_priv);
}

static inline int pwmchip_add(struct pwm_chip *chip)
{
	return -EINVAL;
}

static inline int pwmchip_remove(struct pwm_chip *chip)
{
	return -EINVAL;
}

static inline int devm_pwmchip_add(struct device *dev, struct pwm_chip *chip)
{
	return -EINVAL;
}

static inline struct pwm_device *pwm_get(struct device *dev,
					 const char *consumer)
{
	might_sleep();
	return ERR_PTR(-ENODEV);
}

static inline void pwm_put(struct pwm_device *pwm)
{
	might_sleep();
}

static inline struct pwm_device *devm_pwm_get(struct device *dev,
					      const char *consumer)
{
	might_sleep();
	return ERR_PTR(-ENODEV);
}

static inline struct pwm_device *
devm_fwnode_pwm_get(struct device *dev, struct fwnode_handle *fwnode,
		    const char *con_id)
{
	might_sleep();
	return ERR_PTR(-ENODEV);
}
#endif

static inline void pwm_apply_args(struct pwm_device *pwm)
{
	struct pwm_state state = { };

	

	state.enabled = false;
	state.polarity = pwm->args.polarity;
	state.period = pwm->args.period;
	state.usage_power = false;

	pwm_apply_might_sleep(pwm, &state);
}

struct pwm_lookup {
	struct list_head list;
	const char *provider;
	unsigned int index;
	const char *dev_id;
	const char *con_id;
	unsigned int period;
	enum pwm_polarity polarity;
	const char *module; 
};

#define PWM_LOOKUP_WITH_MODULE(_provider, _index, _dev_id, _con_id,	\
			       _period, _polarity, _module)		\
	{								\
		.provider = _provider,					\
		.index = _index,					\
		.dev_id = _dev_id,					\
		.con_id = _con_id,					\
		.period = _period,					\
		.polarity = _polarity,					\
		.module = _module,					\
	}

#define PWM_LOOKUP(_provider, _index, _dev_id, _con_id, _period, _polarity) \
	PWM_LOOKUP_WITH_MODULE(_provider, _index, _dev_id, _con_id, _period, \
			       _polarity, NULL)

#if IS_ENABLED(CONFIG_PWM)
void pwm_add_table(struct pwm_lookup *table, size_t num);
void pwm_remove_table(struct pwm_lookup *table, size_t num);
#else
static inline void pwm_add_table(struct pwm_lookup *table, size_t num)
{
}

static inline void pwm_remove_table(struct pwm_lookup *table, size_t num)
{
}
#endif

#endif 

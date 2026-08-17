// SPDX-License-Identifier: GPL-2.0-only

#ifndef _LINUX_PSE_CONTROLLER_H
#define _LINUX_PSE_CONTROLLER_H

#include <linux/ethtool.h>
#include <linux/list.h>
#include <uapi/linux/ethtool.h>


#define MAX_PI_CURRENT 1920000

struct phy_device;
struct pse_controller_dev;


struct pse_control_config {
	enum ethtool_podl_pse_admin_state podl_admin_control;
	enum ethtool_c33_pse_admin_state c33_admin_control;
};


struct pse_control_status {
	enum ethtool_podl_pse_admin_state podl_admin_state;
	enum ethtool_podl_pse_pw_d_status podl_pw_status;
	enum ethtool_c33_pse_admin_state c33_admin_state;
	enum ethtool_c33_pse_pw_d_status c33_pw_status;
	u32 c33_pw_class;
	u32 c33_actual_pw;
	struct ethtool_c33_pse_ext_state_info c33_ext_state_info;
	u32 c33_avail_pw_limit;
	struct ethtool_c33_pse_pw_limit_range *c33_pw_limit_ranges;
	u32 c33_pw_limit_nb_ranges;
};


struct pse_controller_ops {
	int (*ethtool_get_status)(struct pse_controller_dev *pcdev,
		unsigned long id, struct netlink_ext_ack *extack,
		struct pse_control_status *status);
	int (*setup_pi_matrix)(struct pse_controller_dev *pcdev);
	int (*pi_is_enabled)(struct pse_controller_dev *pcdev, int id);
	int (*pi_enable)(struct pse_controller_dev *pcdev, int id);
	int (*pi_disable)(struct pse_controller_dev *pcdev, int id);
	int (*pi_get_voltage)(struct pse_controller_dev *pcdev, int id);
	int (*pi_get_pw_limit)(struct pse_controller_dev *pcdev,
			       int id);
	int (*pi_set_pw_limit)(struct pse_controller_dev *pcdev,
			       int id, int max_mW);
};

struct module;
struct device_node;
struct of_phandle_args;
struct pse_control;


enum pse_pi_pairset_pinout {
	ALTERNATIVE_A,
	ALTERNATIVE_B,
};


struct pse_pi_pairset {
	enum pse_pi_pairset_pinout pinout;
	struct device_node *np;
};


struct pse_pi {
	struct pse_pi_pairset pairset[2];
	struct device_node *np;
	struct regulator_dev *rdev;
	bool admin_state_enabled;
};


struct pse_controller_dev {
	const struct pse_controller_ops *ops;
	struct module *owner;
	struct list_head list;
	struct list_head pse_control_head;
	struct device *dev;
	int of_pse_n_cells;
	unsigned int nr_lines;
	struct mutex lock;
	enum ethtool_pse_types types;
	struct pse_pi *pi;
	bool no_of_pse_pi;
};

#if IS_ENABLED(CONFIG_PSE_CONTROLLER)
int pse_controller_register(struct pse_controller_dev *pcdev);
void pse_controller_unregister(struct pse_controller_dev *pcdev);
struct device;
int devm_pse_controller_register(struct device *dev,
				 struct pse_controller_dev *pcdev);

struct pse_control *of_pse_control_get(struct device_node *node);
void pse_control_put(struct pse_control *psec);

int pse_ethtool_get_status(struct pse_control *psec,
			   struct netlink_ext_ack *extack,
			   struct pse_control_status *status);
int pse_ethtool_set_config(struct pse_control *psec,
			   struct netlink_ext_ack *extack,
			   const struct pse_control_config *config);
int pse_ethtool_set_pw_limit(struct pse_control *psec,
			     struct netlink_ext_ack *extack,
			     const unsigned int pw_limit);
int pse_ethtool_get_pw_limit(struct pse_control *psec,
			     struct netlink_ext_ack *extack);

bool pse_has_podl(struct pse_control *psec);
bool pse_has_c33(struct pse_control *psec);

#else

static inline struct pse_control *of_pse_control_get(struct device_node *node)
{
	return ERR_PTR(-ENOENT);
}

static inline void pse_control_put(struct pse_control *psec)
{
}

static inline int pse_ethtool_get_status(struct pse_control *psec,
					 struct netlink_ext_ack *extack,
					 struct pse_control_status *status)
{
	return -EOPNOTSUPP;
}

static inline int pse_ethtool_set_config(struct pse_control *psec,
					 struct netlink_ext_ack *extack,
					 const struct pse_control_config *config)
{
	return -EOPNOTSUPP;
}

static inline int pse_ethtool_set_pw_limit(struct pse_control *psec,
					   struct netlink_ext_ack *extack,
					   const unsigned int pw_limit)
{
	return -EOPNOTSUPP;
}

static inline int pse_ethtool_get_pw_limit(struct pse_control *psec,
					   struct netlink_ext_ack *extack)
{
	return -EOPNOTSUPP;
}

static inline bool pse_has_podl(struct pse_control *psec)
{
	return false;
}

static inline bool pse_has_c33(struct pse_control *psec)
{
	return false;
}

#endif

#endif

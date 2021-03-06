<?php
// $Id$

/**
 * @file
 * Rules integration for the checkout process.
 *
 * @addtogroup rules
 * @{
 */

/**
 * Implements hook_rules_event_info().
 */
function commerce_checkout_rules_event_info() {
  $events = array();

  $events['commerce_checkout_complete'] = array(
    'label' => t('Completing the checkout process'),
    'group' => t('Commerce Checkout'),
    'variables' => array(
      'order' => array(
        'type' => 'commerce_order',
        'label' => t('Completed order', array(), array('context' => 'a drupal commerce order')),
      ),
    ),
    'access callback' => 'commerce_order_rules_access',
  );

  return $events;
}

/**
 * Implements hook_rules_action_info().
 */
function commerce_checkout_rules_action_info() {
  $actions = array();

  $actions['send_account_email'] = array(
    'label' => t('Send account e-mail'),
    'parameter' => array(
      'account' => array(
        'type' => 'user',
        'label' => t('Account'),
      ),
      'email_type' => array(
        'type' => 'text',
        'label' => t('E-mail type'),
        'description' => t("Select the e-mail based on your site's account settings to send to the user."),
        'options list' => 'commerce_checkout_account_email_options',
      ),
    ),
    'group' => t('User'),
    'base' => 'commerce_checkout_action_send_account_email',
    'access callback' => 'rules_system_integration_access',
  );

  return $actions;
}

/**
 * Returns the account e-mail types from the User module.
 *
 * @see _user_mail_notify()
 */
function commerce_checkout_account_email_options() {
  return array(
    'register_admin_created' => t('Welcome (new user created by administrator)'),
    'register_no_approval_required' => t('Welcome (no approval required)'),
    'register_pending_approval' => t('Welcome (awaiting approval)'),
    'password_reset' => t('Password recovery'),
    'status_activated' => t('Account activation'),
    'status_blocked' => t('Account blocked'),
    'cancel_confirm' => t('Account cancellation confirmation'),
    'status_canceled' => t('Account canceled'),
  );
}

/**
 * Action callback: sends a selected account e-mail.
 */
function commerce_checkout_action_send_account_email($account, $email_type) {
  // If we received an authenticated user account...
  if (!empty($account)) {
    $types = commerce_checkout_account_email_options();

    // Attempt to send the account e-mail.
    $result = _user_mail_notify($email_type, $account);

    // Log the success or failure.
    if ($result) {
      watchdog('rules', '%type e-mail sent to %recipient.', array('%type' => $types[$email_type], '%recipient' => $account->mail));
    }
    else {
      watchdog('rules', 'Failed to send %type e-mail to %recipient.', array('%type' => $types[$email_type], '%recipient' => $account->mail));
    }
  }
}

/**
 * Implements hook_rules_condition_info().
 */
function commerce_checkout_rules_condition_info() {
  $conditions = array();

  module_load_include('inc', 'rules', 'data.rules');

  $conditions['entity_exists'] = array(
    'label' => t('Entity exists by property'),
    'parameter' => array(
      'type' => array(
        'type' => 'text',
        'label' => t('Entity type'),
        'options list' => 'commerce_checkout_entity_type_options',
        'description' => t('Specifies the type of the entity that should be fetched.'),
        'restriction' => 'input',
       ),
      'property' => array(
        'type' => 'text',
        'label' => t('Property'),
        'description' => t('The property by which the entity is to be selected.'),
        'restriction' => 'input',
       ),
      'value' => array(
        'type' => 'text',
        'label' => t('Value'),
        'description' => t('The property value of the entity to be fetched.'),
       ),
    ),
    'group' => t('Entities'),
    'base' => 'commerce_checkout_condition_entity_exists',
    'callbacks' => array(
      'form_alter' => 'rules_action_type_form_alter',
    ),
  );

  return $conditions;
}

/**
 * Returns options containing entity types having the given key set in the info.
 */
function commerce_checkout_entity_type_options() {
  $types = array();

  foreach (entity_get_info() as $type => $entity_info) {
    $types[$type] = $entity_info['label'];
  }

  return $types;
}

/**
 * Condition callback: checks to see if an entity exists with the matching
 *   property value.
 */
function commerce_checkout_condition_entity_exists($type, $property, $value) {
  // TODO: This can be replaced with a call to entity_metadata_table_query()
  // when http://drupal.org/node/1041240 gets fixed.
  $result = commerce_checkout_entity_metadata_table_query($type, $property, $value, 1);
  return !empty($result);
}

/**
 * @}
 */

<?php

namespace RESTAPI\Dispatchers;

use RESTAPI\Core\Command;
use RESTAPI\Core\Dispatcher;

/**
 * Defines a Dispatcher for applying changes to the firewall filter configuration.
 */
class FirewallApplyDispatcher extends Dispatcher {
    /**
     * Reloads the filter services.
     */
    protected function _process(mixed ...$arguments): void {
        new Command("/etc/rc.filter_configure_sync");
    }
}

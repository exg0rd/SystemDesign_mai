#!/bin/bash

echo "Service type: $SERVICE_TYPE"

if [ "$SERVICE_TYPE" = "consumer" ]; then
    echo "Starting event consumer service..."
    ./build/event_manager --config configs/static_config.consumer.yaml --config_vars configs/config_vars.yaml
else
    echo "Starting API server (default mode)..."
    ./build/event_manager --config configs/static_config.api.yaml --config_vars configs/config_vars.yaml
fi
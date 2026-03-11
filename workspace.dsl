workspace "Event Management System" "Система управления событиями - вариант 22" {

    model {
        # Определение пользователей
        organizer = person "Организатор событий" "Создает и управляет событиями"
        participant = person "Участник" "Регистрируется на события и управляет своими регистрациями"
        admin = person "Администратор" "Управляет пользователями и системой"
        
        # Определение внешних систем
        emailService = softwareSystem "Email Service" "Отправка email-уведомлений пользователям" "External"
        smsService = softwareSystem "SMS Service" "Отправка SMS-уведомлений участникам" "External"
        paymentSystem = softwareSystem "Payment System" "Обработка платежей за платные события" "External"
        calendarService = softwareSystem "Calendar Service" "Интеграция с календарями пользователей" "External"
        
        # Определение основной системы
        eventManagementSystem = softwareSystem "Event Management System" "Система управления событиями и регистрацией участников" {
            
            # Контейнеры системы
            webApp = container "Web Application" "Предоставляет пользовательский интерфейс для работы с событиями" "React/Vue.js" "Web Browser"
            
            apiGateway = container "API Gateway" "Точка входа для всех API запросов, маршрутизация и аутентификация" "Node.js/Express" "API"
            
            userService = container "User Service" "Управление пользователями, аутентификация и авторизация" "Node.js/TypeScript" "Service"
            
            eventService = container "Event Service" "Управление событиями: создание, поиск, обновление" "Node.js/TypeScript" "Service"
            
            participantService = container "Participant Service" "Управление регистрациями пользователей на события" "Node.js/TypeScript" "Service"
            
            notificationService = container "Notification Service" "Отправка уведомлений пользователям через различные каналы" "Node.js/TypeScript" "Service"
            
            database = container "Database" "Хранит данные пользователей, событий и регистраций" "PostgreSQL" "Database"
            
            cache = container "Cache" "Кэширует списки событий и данные пользователей" "Redis" "Cache"
            
            messageQueue = container "Message Queue" "Асинхронная обработка уведомлений" "RabbitMQ" "Queue"
        }
        
        # Взаимодействие пользователей с системой
        organizer -> webApp "Создает и управляет событиями" "HTTPS"
        participant -> webApp "Просматривает события и регистрируется" "HTTPS"
        admin -> webApp "Управляет пользователями и системой" "HTTPS"
        
        # Взаимодействие Web Application с API Gateway
        webApp -> apiGateway "Отправляет API запросы" "HTTPS/REST"
        
        # Взаимодействие API Gateway с сервисами
        apiGateway -> userService "Маршрутизирует запросы пользователей" "HTTPS/REST"
        apiGateway -> eventService "Маршрутизирует запросы событий" "HTTPS/REST"
        apiGateway -> participantService "Маршрутизирует запросы регистраций" "HTTPS/REST"
        
        # Взаимодействие между сервисами
        participantService -> userService "Проверяет существование пользователя" "HTTPS/REST"
        participantService -> eventService "Проверяет существование события" "HTTPS/REST"
        participantService -> messageQueue "Публикует события регистрации" "AMQP"
        
        notificationService -> messageQueue "Подписывается на события" "AMQP"
        
        # Взаимодействие сервисов с базой данных
        userService -> database "Читает и записывает данные пользователей" "JDBC/SQL"
        eventService -> database "Читает и записывает данные событий" "JDBC/SQL"
        participantService -> database "Читает и записывает данные регистраций" "JDBC/SQL"
        
        # Взаимодействие с кэшем
        eventService -> cache "Кэширует списки событий" "Redis Protocol"
        userService -> cache "Кэширует данные пользователей" "Redis Protocol"
        
        # Взаимодействие с внешними системами
        notificationService -> emailService "Отправляет email-уведомления" "SMTP/API"
        notificationService -> smsService "Отправляет SMS-уведомления" "HTTPS/REST"
        eventService -> calendarService "Синхронизирует события с календарями" "HTTPS/REST"
        participantService -> paymentSystem "Обрабатывает платежи за события" "HTTPS/REST"
        
        # Deployment окружение PROD
        deploymentEnvironment "PROD" {
            deploymentNode "User Device" "" "User's Computer/Mobile" {
                deploymentNode "Web Browser" "" "Chrome, Firefox, Safari" {
                    webAppInstance = containerInstance webApp
                }
            }
            
            deploymentNode "Cloud Provider" "" "AWS/Azure/GCP" {
                deploymentNode "Load Balancer" "" "AWS ALB/Nginx" {
                    tags "Infrastructure"
                }
                
                deploymentNode "Kubernetes Cluster" "" "K8s" {
                    deploymentNode "Frontend Pod" "" "Docker Container" {
                        deploymentNode "Nginx" "" "Web Server" {
                            webAppStaticInstance = containerInstance webApp
                        }
                    }
                    
                    deploymentNode "API Gateway Pod" "" "Docker Container" {
                        apiGatewayInstance = containerInstance apiGateway
                    }
                    
                    deploymentNode "User Service Pod" "" "Docker Container" {
                        deploymentNode "Node.js Runtime" "" "v18 LTS" {
                            userServiceInstance = containerInstance userService
                        }
                    }
                    
                    deploymentNode "Event Service Pod" "" "Docker Container" {
                        deploymentNode "Node.js Runtime" "" "v18 LTS" {
                            eventServiceInstance = containerInstance eventService
                        }
                    }
                    
                    deploymentNode "Participant Service Pod" "" "Docker Container" {
                        deploymentNode "Node.js Runtime" "" "v18 LTS" {
                            participantServiceInstance = containerInstance participantService
                        }
                    }
                    
                    deploymentNode "Notification Service Pod" "" "Docker Container" {
                        deploymentNode "Node.js Runtime" "" "v18 LTS" {
                            notificationServiceInstance = containerInstance notificationService
                        }
                    }
                }
                
                deploymentNode "Database Server" "" "AWS RDS/Managed PostgreSQL" {
                    deploymentNode "PostgreSQL Primary" "" "PostgreSQL 15" {
                        databaseInstance = containerInstance database
                    }
                    deploymentNode "PostgreSQL Replica" "" "PostgreSQL 15 Read Replica" {
                        tags "Database"
                    }
                }
                
                deploymentNode "Cache Server" "" "AWS ElastiCache/Managed Redis" {
                    deploymentNode "Redis Cluster" "" "Redis 7" {
                        cacheInstance = containerInstance cache
                    }
                }
                
                deploymentNode "Message Broker" "" "AWS MQ/Managed RabbitMQ" {
                    deploymentNode "RabbitMQ Cluster" "" "RabbitMQ 3.12" {
                        messageQueueInstance = containerInstance messageQueue
                    }
                }
            }
            
            deploymentNode "External Services" "" "Third-party providers" {
                deploymentNode "SendGrid" "" "Email Service Provider" {
                    emailServiceInstance = softwareSystemInstance emailService
                }
                deploymentNode "Twilio" "" "SMS Service Provider" {
                    smsServiceInstance = softwareSystemInstance smsService
                }
                deploymentNode "Stripe" "" "Payment Processor" {
                    paymentSystemInstance = softwareSystemInstance paymentSystem
                }
                deploymentNode "Google Calendar API" "" "Calendar Integration" {
                    calendarServiceInstance = softwareSystemInstance calendarService
                }
            }
        }
    }

    views {
        # System Context диаграмма (C1)
        systemContext eventManagementSystem "SystemContext" {
            include *
            autoLayout
            description "Диаграмма контекста системы управления событиями"
        }
        
        # Container диаграмма (C2)
        container eventManagementSystem "Containers" {
            include *
            autoLayout
            description "Диаграмма контейнеров системы управления событиями"
        }
        
        # Deployment диаграмма (C3)
        deployment eventManagementSystem "PROD" "Deployment" {
            include *
            autoLayout
            description "Диаграмма развертывания системы в production окружении"
        }
        
        # Dynamic диаграмма - сценарий регистрации на событие
        dynamic eventManagementSystem "UserRegistration" "Сценарий регистрации пользователя на событие" {
            participant -> webApp "1. Выбирает событие и нажимает 'Зарегистрироваться'"
            webApp -> apiGateway "2. POST /api/events/{eventId}/participants"
            apiGateway -> participantService "3. Перенаправляет запрос регистрации"
            participantService -> userService "4. GET /api/users/{userId} - проверяет пользователя"
            userService -> database "5. SELECT * FROM users WHERE id = ?"
            database -> userService "6. Возвращает данные пользователя"
            userService -> participantService "7. Возвращает подтверждение существования"
            participantService -> eventService "8. GET /api/events/{eventId} - проверяет событие"
            eventService -> database "9. SELECT * FROM events WHERE id = ?"
            database -> eventService "10. Возвращает данные события"
            eventService -> participantService "11. Возвращает подтверждение существования"
            participantService -> database "12. INSERT INTO participants (user_id, event_id)"
            database -> participantService "13. Подтверждает создание записи"
            participantService -> messageQueue "14. Публикует событие 'UserRegistered'"
            messageQueue -> notificationService "15. Доставляет событие"
            notificationService -> emailService "16. Отправляет подтверждение регистрации"
            emailService -> notificationService "17. Подтверждает отправку"
            participantService -> apiGateway "18. Возвращает успешный ответ"
            apiGateway -> webApp "19. Возвращает статус 201 Created"
            webApp -> participant "20. Показывает подтверждение регистрации"
            autoLayout
        }
        
        # Стили для элементов
        styles {
            element "Person" {
                shape person
                background #08427b
                color #ffffff
            }
            element "External" {
                background #999999
                color #ffffff
            }
            element "Web Browser" {
                shape WebBrowser
                background #438dd5
                color #ffffff
            }
            element "API" {
                shape RoundedBox
                background #438dd5
                color #ffffff
            }
            element "Service" {
                shape Hexagon
                background #85bbf0
                color #000000
            }
            element "Database" {
                shape Cylinder
                background #438dd5
                color #ffffff
            }
            element "Cache" {
                shape Cylinder
                background #ff6b6b
                color #ffffff
            }
            element "Queue" {
                shape Pipe
                background #f9ca24
                color #000000
            }
            element "Infrastructure" {
                shape RoundedBox
                background #cccccc
                color #000000
            }
        }
    }
    
    configuration {
        scope softwaresystem
    }
}

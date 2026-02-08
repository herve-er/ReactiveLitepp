#include <iostream>
#include <string>
#include <vector>
#include <ReactiveLitepp/Event.h>

using namespace ReactiveLitepp;

int main() {
    std::cout << "=== ReactiveLitepp Test ===" << std::endl << std::endl;

    // Create events of different types
    Event<std::string, int> messageEvent;
    Event<double> valueEvent;
    Event<> simpleEvent;
    
    // Store subscriptions from different event types in the same vector!
    std::vector<Subscription> subscriptions;
    
    subscriptions.push_back(messageEvent.Subscribe([](const std::string& msg, int count) {
        std::cout << "Message: " << msg << " (count: " << count << ")" << std::endl;
    }));
    
    subscriptions.push_back(valueEvent.Subscribe([](double value) {
        std::cout << "Value: " << value << std::endl;
    }));
    
    subscriptions.push_back(simpleEvent.Subscribe([]() {
        std::cout << "Simple event triggered!" << std::endl;
    }));
    
    std::cout << "Test 1: Notify all events" << std::endl;
    messageEvent.Notify("Hello", 1);
    valueEvent.Notify(3.14);
    simpleEvent.Notify();
    
    std::cout << "\nTest 2: Unsubscribe first subscription" << std::endl;
    subscriptions[0].Unsubscribe();
    
    messageEvent.Notify("After unsubscribe", 2);
    valueEvent.Notify(2.71);
    simpleEvent.Notify();
    
    std::cout << "\nTest 3: Check subscription validity" << std::endl;
    for (size_t i = 0; i < subscriptions.size(); ++i) {
        std::cout << "Subscription " << i << " is " 
                  << (subscriptions[i].IsValid() ? "valid" : "invalid") << std::endl;
    }
    
    std::cout << "\nTest 4: Scoped subscriptions" << std::endl;
    {
        std::vector<ScopedSubscription> scopedSubs;
        scopedSubs.push_back(messageEvent.SubscribeScoped([](const std::string& msg, int count) {
            std::cout << "Scoped handler: " << msg << std::endl;
        }));
        
        messageEvent.Notify("Inside scope", 3);
    } // scopedSubs destructed here, automatically unsubscribes
    
    std::cout << "After scope ended:" << std::endl;
    messageEvent.Notify("Outside scope", 4);
    
    std::cout << "\n=== Test completed ===" << std::endl;
    
    return 0;
}
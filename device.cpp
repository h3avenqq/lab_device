/**
 * @file main.cpp
 *
 * @brief A C++ program demonstrating the usage of the Stream and Device classes.
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <gtest/gtest.h>

using namespace std;

int streamcounter; ///< Global variable to keep track of stream creation.

const int MIXER_OUTPUTS = 1;
const float POSSIBLE_ERROR = 0.01;

/**
 * @class Stream
 * @brief Represents a chemical stream with a name and mass flow.
 */
class Stream
{
private:
    double mass_flow; ///< The mass flow rate of the stream.
    string name;      ///< The name of the stream.

public:
    /**
     * @brief Constructor to create a Stream with a unique name.
     * @param s An integer used to generate a unique name for the stream.
     */
    Stream(int s){setName("s"+std::to_string(s));}

    /**
     * @brief Set the name of the stream.
     * @param s The new name for the stream.
     */
    void setName(string s){name=s;}

    /**
     * @brief Get the name of the stream.
     * @return The name of the stream.
     */
    string getName(){return name;}

    /**
     * @brief Set the mass flow rate of the stream.
     * @param m The new mass flow rate value.
     */
    void setMassFlow(double m){mass_flow=m;}

    /**
     * @brief Get the mass flow rate of the stream.
     * @return The mass flow rate of the stream.
     */
    double getMassFlow() const {return mass_flow;}

    /**
     * @brief Print information about the stream.
     */
    void print() { cout << "Stream " << getName() << " flow = " << getMassFlow() << endl; }
};

/**
 * @class Device
 * @brief Represents a device that manipulates chemical streams.
 */
class Device
{
protected:
    int inputAmount;
    int outputAmount;
    bool isCalculated;
public:
    vector<shared_ptr<Stream>> inputs;  ///< Input streams connected to the device.
    vector<shared_ptr<Stream>> outputs; ///< Output streams produced by the device.

    Device(){
        isCalculated = false;
    }

    /**
     * @brief Add an input stream to the device.
     * @param s A shared pointer to the input stream.
     */
    void addInput(shared_ptr<Stream> s){
      if(inputs.size() < inputAmount) inputs.push_back(s);
      else throw "INPUT STREAM LIMIT!";
    }
    /**
     * @brief Add an output stream to the device.
     * @param s A shared pointer to the output stream.
     */
    void addOutput(shared_ptr<Stream> s){
      if(outputs.size() < outputAmount) outputs.push_back(s);
      else throw "OUTPUT STREAM LIMIT!";
    }

    /**
     * @brief Update the output streams of the device (to be implemented by derived classes).
     */
    virtual void updateOutputs() = 0;

    bool isDeviceCalculated(){
        return isCalculated;
    }
};

class Mixer: public Device
{
    private:
      int _inputs_count = 0;
    public:
        Mixer(int inputs_count): Device() {
            _inputs_count = inputs_count;
        }

        void addInput(shared_ptr<Stream> s) {
        if (inputs.size() == _inputs_count) {
          throw "Too much inputs"s;
        }
        inputs.push_back(s);
        }

        void addOutput(shared_ptr<Stream> s) {
        if (outputs.size() == MIXER_OUTPUTS) {
          throw "Too much outputs"s;
        }
        outputs.push_back(s);
        }

        void updateOutputs() override {
            if (isCalculated)
                throw std::runtime_error("Device is already calculated");

            double sum_mass_flow = 0;
            for (const auto& input_stream : inputs) {
                sum_mass_flow += input_stream -> getMassFlow();
            }

            if (outputs.empty()) {
                throw "Should set outputs before update"s;
            }

            double output_mass = sum_mass_flow / outputs.size(); // divide 0

            for (auto& output_stream : outputs) {
                output_stream -> setMassFlow(output_mass);
            }

            isCalculated = true;
        }
};

void shouldSetOutputsCorrectlyWithOneOutput() {
    streamcounter=0;
    Mixer d1 = Mixer(2);

    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);

    d1.addInput(s1);
    d1.addInput(s2);
    d1.addOutput(s3);

    d1.updateOutputs();

    if (abs(s3->getMassFlow()) - 15 < POSSIBLE_ERROR) {
      cout << "Test 1 passed"s << endl;
    } else {
      cout << "Test 1 failed"s << endl;
    }
}

void shouldCorrectOutputs() {
    streamcounter=0;
    Mixer d1 = Mixer(2);

    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    shared_ptr<Stream> s4(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);

    d1.addInput(s1);
    d1.addInput(s2);
    d1.addOutput(s3);

    try {
      d1.addOutput(s4);
    } catch (const string ex) {
      if (ex == "Too much outputs"s) {
        cout << "Test 2 passed"s << endl;

        return;
      }
    }

    cout << "Test 2 failed"s << endl;
}

void shouldCorrectInputs() {
    streamcounter=0;
    Mixer d1 = Mixer(2);

    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    shared_ptr<Stream> s4(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);

    d1.addInput(s1);
    d1.addInput(s2);
    d1.addOutput(s3);

    try {
      d1.addInput(s4);
    } catch (const string ex) {
      if (ex == "Too much inputs"s) {
        cout << "Test 3 passed"s << endl;

        return;
      }
    }

    cout << "Test 3 failed"s << endl;
}

class Reactor : public Device{
public:
    Reactor(bool isDoubleReactor) {
        inputAmount = 1;
        if (isDoubleReactor) outputAmount = 2;
        else outputAmount = 1;

        cout << this->inputAmount << " " << this->outputAmount << endl;
    }

    void updateOutputs() override{
        if (isCalculated)
            throw std::runtime_error("Device is already calculated");

        double inputMass = inputs.at(0)->getMassFlow();
        for(int i = 0; i < outputs.size(); i++){
            double outputLocal = inputMass * (1/outputAmount);
            outputs.at(i) -> setMassFlow(outputLocal);
        }

        isCalculated = true;
    }
};

void testTooManyOutputStreams(){
    streamcounter=0;
    
    Reactor dl = new Reactor(false);
    
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);
    dl.addInput(s1);
    dl.addOutput(s2);
    try{
        dl.addOutput(s3);
    } catch(const string ex){
         if (ex == "OUTPUT STREAM LIMIT!")
            cout << "Test 1 passed" << endl;

        return;
    }
    
     cout << "Test 1 failed" << endl;
}

void testTooManyInputStreams(){
    streamcounter=0;
    
    Reactor dl = new Reactor(false);
    
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s3->setMassFlow(5.0);
    dl.addInput(s1);
    try{
        dl.addInput(s3);
    } catch(const string ex){
         if (ex == "INPUT STREAM LIMIT!")
            cout << "Test 2 passed" << endl;

        return;
    }
    
     cout << "Test 2 failed"s << endl;
}

void testInputEqualOutput(){
    streamcounter=0;
    
    Reactor dl = new Reactor(true);
    
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);
    dl.addInput(s1);
    dl.addOutput(s2);
    dl.addOutput(s3);
    
    dl.updateOutputs();
    
    if(dl.outputs.at(0)->getMassFlow() + dl.outputs.at(1)->getMassFlow() == dl.inputs.at(0)->getMassFlow())
        cout << "Test 3 passed" << endl;
    else
        cout << "Test 3 failed" << endl;
}

void tests(){
    testInputEqualOutput();
    testTooManyOutputStreams();
    testTooManyInputStreams();
    
    shouldSetOutputsCorrectlyWithOneOutput();
    shouldCorrectOutputs();
    shouldCorrectInputs();
}

TEST(DeviceTests, TooMuchOutputStreamsException){
    // Arrange
    streamcounter=0;

    Reactor dl = new Reactor(false);

    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));

    // Act
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);
    dl.addInput(s1);
    dl.addOutput(s2);

    // Assert
    try{
        dl.addOutput(s3);
    } catch(const string ex){
        EXPECT_EQ(ex ,std::string("OUTPUT STREAM LIMIT!"));
    } catch (...) {
        FAIL();
    }

}

TEST(DeviceTests, TooMuchInputStreamsException){
    // Arrange
    streamcounter=0;

    Reactor dl = new Reactor(false);

    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));

    // Act
    s1->setMassFlow(10.0);
    s2->setMassFlow(15.0);
    dl.addInput(s1);

    // Assert
    try {
        dl.addInput(s2);
    } catch (...) {
        SUCCEED();
    }

}

TEST(DeviceTests, SetMassFlow){
    // Arrange
    streamcounter=0;

    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));

    // Act
    s1->setMassFlow(15.0);
    s2->setMassFlow(5.0);
    s3->setMassFlow(10.0);


    // Assert
    ASSERT_TRUE(s2->getMassFlow() == 5.0 &&
        s3->getMassFlow() == 10.0 &&
        s1->getMassFlow() == 15.0);
}

TEST(DeviceTests, SetNameOfStream){
    // Arrange
    shared_ptr<Stream> s1(new Stream(1));

    // Act
    s1->setName("New name");
    string result = s1->getName();

    // Assert
    EXPECT_EQ(result, "New name");
}

TEST(DeviceTests, DeviceIsCalculated){
    // Arrange
    streamcounter=0;

    Reactor dl = new Reactor(false);

    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));

    // Act
    s1->setMassFlow(10.0);
    s2->setMassFlow(15.0);
    dl.addInput(s1);
    dl.addOutput(s2);
    dl.updateOutputs();
    // Assert
    try {
        dl.updateOutputs();
        FAIL() << "Expected std::runtime_error";
    } catch (std::runtime_error const & err) {
        EXPECT_EQ(err.what(), std::string("Device is already calculated"));
    } catch (...) {
        FAIL() << "Expected std::runtime_error";
    }
    cout << dl.isDeviceCalculated() << endl;
}

/**
 * @brief The entry point of the program.
 * @return 0 on successful execution.
 */
int main(int argc, char **argv)
{
    streamcounter = 0;

    // Create streams
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));

    // Set mass flows
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);

    // Create a device (e.g., Mixer) and add input/output streams
    // Mixer d1;
    // d1.addInput(s1);
    // d1.addInput(s2);
    // d1.addOutput(s3);

    // Update the outputs of the device
    // d1.updateOutputs();

    // Print stream information
//    s1->print();
//    s2->print();
//    s3->print();
//    tests();

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

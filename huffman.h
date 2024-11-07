#ifndef HUFFMANAPP_H
#define HUFFMANAPP_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QString>

// Forward declaration for HuffmanNode struct
struct HuffmanNode;

class HuffmanApp : public QMainWindow {
    Q_OBJECT

public:
    explicit HuffmanApp(QWidget *parent = nullptr);
    ~HuffmanApp();

private slots:
    void loadFile();        // Slot to load a text file
    void encodeText();      // Slot to encode the input text using Huffman encoding
    void decodeText();      // Slot to decode the encoded text
    void saveFile();        // Slot to save the encoded or decoded text
    void printHuffmanDictionary();  // Slot to print and save the Huffman dictionary
    void compareTextCompressionAlgorithms();

private:
    QString inputText;      // Stores the input text for encoding
    QString encodedText;    // Stores the encoded text
    QString decodedText;    // Stores the decoded text
    QString dictionaryText; // Stores the dictionary text

    // UI Elements
    QTextEdit *inputTextArea;     // Text area for input text
    QTextEdit *encodedTextArea;   // Text area for encoded text
    QTextEdit *decodedTextArea;   // Text area for decoded text
    QPushButton *loadButton;      // Button to load a file
    QPushButton *encodeButton;    // Button to encode text
    QPushButton *decodeButton;    // Button to decode text
    QPushButton *saveButton;      // Button to save the text
    QPushButton *compareAlgorithmsButton;

    // Helper functions
    void generateHuffmanCodes(HuffmanNode* node, QString code, QMap<QChar, QString>& huffmanCodes);
    void deleteTree(HuffmanNode* node);
};

// Definition of HuffmanNode struct
struct HuffmanNode {
    QChar character;
    int frequency;
    HuffmanNode *left;
    HuffmanNode *right;

    HuffmanNode(QChar ch, int freq) : character(ch), frequency(freq), left(nullptr), right(nullptr) {}
};

// Comparator for priority queue (std::priority_queue)
struct CompareFrequency {
    bool operator()(HuffmanNode* left, HuffmanNode* right) {
        return left->frequency > right->frequency;
    }
};

#endif // HUFFMANAPP_H

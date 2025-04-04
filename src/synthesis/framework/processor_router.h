/**
 * @file processor_router.h
 * @brief Declares the ProcessorRouter class, which manages a graph of Processors and their dependencies.
 *
 * The ProcessorRouter is responsible for coordinating multiple Processors in a signal processing chain.
 * It manages the order in which Processors run (topological sorting), ensures that feedback loops are
 * handled via Feedback objects, and handles both polyphonic and monophonic processing paths.
 *
 * By nesting ProcessorRouters, complex signal routing and modular arrangements of Processors can be built.
 */
#pragma once

#include "processor.h"
#include "circular_queue.h"

#include <map>
#include <set>
#include <vector>

namespace vital {

  class Feedback;

  /**
   * @class ProcessorRouter
   * @brief A specialized Processor that manages a directed graph of Processors and ensures correct processing order.
   *
   * ProcessorRouter maintains a list of Processors and Feedback objects, and ensures that the audio/control graph
   * is executed in the correct order each audio block. It manages detection of cycles, insertion of Feedback nodes,
   * and propagation of sample rate and oversampling changes.
   *
   * This class is central to Vital's flexible and modular signal routing architecture.
   */
  class ProcessorRouter : public Processor {
    public:
      /**
       * @brief Constructs a ProcessorRouter with a specified number of inputs and outputs.
       * @param num_inputs Number of initial inputs.
       * @param num_outputs Number of initial outputs.
       * @param control_rate Whether this router runs at control rate or not.
       */
      ProcessorRouter(int num_inputs = 0, int num_outputs = 0, bool control_rate = false);

      /**
       * @brief Copy constructor. Creates a new ProcessorRouter from an existing one.
       * @param original The ProcessorRouter to copy.
       */
      ProcessorRouter(const ProcessorRouter& original);

      /**
       * @brief Destructor.
       */
      virtual ~ProcessorRouter();

      /**
       * @brief Creates a copy of this ProcessorRouter.
       * @return A dynamically allocated copy of this ProcessorRouter.
       */
      virtual Processor* clone() const override {
        return new ProcessorRouter(*this);
      }

      /**
       * @brief Processes audio through all Processors managed by this router.
       * @param num_samples The number of samples to process.
       *
       * This ensures all Feedback loops are refreshed and then processes all Processors
       * in the correct order, respecting sample rates and oversampling factors.
       */
      virtual void process(int num_samples) override;

      /**
       * @brief Initializes the ProcessorRouter and all its Processors.
       */
      virtual void init() override;

      /**
       * @brief Sets the sample rate for all Processors in this router.
       * @param sample_rate The sample rate in Hz.
       *
       * Also updates the sample rates of all child Processors and Feedback loops.
       */
      virtual void setSampleRate(int sample_rate) override;

      /**
       * @brief Sets the oversampling amount for all Processors in this router.
       * @param oversample The oversampling factor.
       *
       * Updates all Processors and Feedback nodes accordingly.
       */
      virtual void setOversampleAmount(int oversample) override;

      /**
       * @brief Adds a Processor to be managed by this router.
       * @param processor The Processor to add.
       *
       * The Processor becomes part of the routing graph. The router ensures that
       * the Processor is run at the appropriate point in the signal flow.
       */
      virtual void addProcessor(Processor* processor);

      /**
       * @brief Adds a Processor to the router in real-time (no memory allocations).
       * @param processor The Processor to add.
       */
      virtual void addProcessorRealTime(Processor* processor);

      /**
       * @brief Adds a Processor that should remain idle (not processed) in the router.
       * @param processor The idle Processor to add.
       */
      virtual void addIdleProcessor(Processor* processor);

      /**
       * @brief Removes a Processor from this router.
       * @param processor The Processor to remove.
       */
      virtual void removeProcessor(Processor* processor);

      // Any time new dependencies are added into the ProcessorRouter graph, we
      // should call _connect_ on the destination Processor and source Output.
      /**
       * @brief Connects a source Output to a destination Processor input by index.
       * @param destination The Processor to connect to.
       * @param source The Output to connect from.
       * @param index The input index on the destination Processor.
       *
       * If introducing a cycle, a Feedback node is created to handle it.
       */
      void connect(Processor* destination, const Output* source, int index);

      /**
       * @brief Disconnects a source Output from a destination Processor.
       * @param destination The Processor that was receiving the connection.
       * @param source The source Output that was connected.
       */
      void disconnect(const Processor* destination, const Output* source);

      /**
       * @brief Checks if one Processor is downstream from another, i.e., if there's a path from the second to the first.
       * @param first The Processor to check downstream.
       * @param second The Processor from which paths are checked.
       * @return True if first is downstream of second, false otherwise.
       */
      bool isDownstream(const Processor* first, const Processor* second) const;

      /**
       * @brief Checks if the order of two Processors is fixed in the router's processing sequence.
       * @param first The first Processor.
       * @param second The second Processor.
       * @return True if first is processed before second in the current order, false otherwise.
       */
      bool areOrdered(const Processor* first, const Processor* second) const;

      /**
       * @brief Determines if a given Processor is polyphonic within this router.
       * @param processor The Processor to check.
       * @return True if the Processor is polyphonic, false otherwise.
       */
      virtual bool isPolyphonic(const Processor* processor) const;

      /**
       * @brief Gets the mono router that corresponds to this ProcessorRouter.
       * @return A pointer to a mono version of this router, if available, otherwise this router.
       */
      virtual ProcessorRouter* getMonoRouter();

      /**
       * @brief Gets the polyphonic router that corresponds to this ProcessorRouter.
       * @return A pointer to the poly version of this router, which may be this router itself.
       */
      virtual ProcessorRouter* getPolyRouter();

      /**
       * @brief Resets all Feedback nodes within this router using a reset mask.
       * @param reset_mask A poly_mask indicating which voices to reset.
       */
      virtual void resetFeedbacks(poly_mask reset_mask);

    protected:
      // When we create a cycle into the ProcessorRouter graph, we must insert
      // a Feedback node and add it here.
      /**
       * @brief Adds a Feedback node to handle a feedback loop introduced by a connection.
       * @param feedback The Feedback node to add.
       */
      virtual void addFeedback(Feedback* feedback);

      /**
       * @brief Removes a previously added Feedback node.
       * @param feedback The Feedback node to remove.
       */
      virtual void removeFeedback(Feedback* feedback);

      // Makes sure _processor_ runs in a topologically sorted order in
      // relation to all other Processors in _this_.
      /**
       * @brief Reorders the internal processing sequence to account for a Processor's dependencies.
       * @param processor The Processor that may require reordering.
       */
      void reorder(Processor* processor);

      // Ensures our local copies of all processors and feedback processors match the master order.
      /**
       * @brief Updates all processors to match the global order. Called when changes occur.
       */
      virtual void updateAllProcessors();

      /**
       * @brief Checks if local changes need to be synchronized with global changes.
       * @return True if updates are required, false otherwise.
       */
      force_inline bool shouldUpdate() { return local_changes_ != *global_changes_; }

      // Will create local copies of added processors.
      /**
       * @brief Creates any processors that were added at the global level but not yet replicated locally.
       */
      virtual void createAddedProcessors();

      // Will delete local copies of removed processors.
      /**
       * @brief Deletes any processors that were removed at the global level but not yet removed locally.
       */
      virtual void deleteRemovedProcessors();

      // Returns the ancestor of _processor_ which is a child of _this_.
      // Returns null if _processor_ is not a descendant of _this_.
      /**
       * @brief Gets the processor context within this router for a global Processor reference.
       * @param processor The Processor to find the local context for.
       * @return The local Processor pointer or nullptr if not found.
       */
      const Processor* getContext(const Processor* processor) const;

      /**
       * @brief Populates the internal dependencies structure for a given Processor.
       * @param processor The Processor to analyze dependencies for.
       */
      void getDependencies(const Processor* processor) const;

      // Returns the processor for this voice from the globally created one.
      /**
       * @brief Retrieves the local instance of a globally defined Processor.
       * @param global_processor The global Processor pointer.
       * @return A pointer to the local Processor instance.
       */
      Processor* getLocalProcessor(const Processor* global_processor);

      std::shared_ptr<CircularQueue<Processor*>> global_order_;       ///< Global processing order reference.
      std::shared_ptr<CircularQueue<Processor*>> global_reorder_;     ///< Temporary storage for reorder operations.
      CircularQueue<Processor*> local_order_;                          ///< Local ordering of Processors.

      std::map<const Processor*, std::pair<int, std::unique_ptr<Processor>>> processors_; ///< Map of global to local Processors.
      std::map<const Processor*, std::unique_ptr<Processor>> idle_processors_;            ///< Idle Processors that are not active in the graph.

      std::shared_ptr<std::vector<const Feedback*>> global_feedback_order_;              ///< Global order of Feedback nodes.
      std::vector<Feedback*> local_feedback_order_;                                      ///< Local copies of Feedback nodes.
      std::map<const Processor*, std::pair<int, std::unique_ptr<Feedback>>> feedback_processors_; ///< Map of global to local Feedback processors.

      std::shared_ptr<int> global_changes_;  ///< Global change counter.
      int local_changes_;                    ///< Local change counter to track synchronization with global changes.

      std::shared_ptr<CircularQueue<const Processor*>> dependencies_;         ///< Queue for dependencies calculations.
      std::shared_ptr<CircularQueue<const Processor*>> dependencies_visited_; ///< Queue of visited processors for dependencies calc.
      std::shared_ptr<CircularQueue<const Processor*>> dependency_inputs_;     ///< Queue of processors to check inputs for dependencies.

      JUCE_LEAK_DETECTOR(ProcessorRouter)
  };
} // namespace vital

